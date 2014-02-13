/*
 * app_gyroReader.c
 *
 *  Created on: Oct 23, 2012
 *      Author: lutz
 */


#include <flawless/stdtypes.h>
#include <flawless/core/msg_msgPump.h>
#include <flawless/config/msgIDs.h>

#include <flawless/init/systemInitializer.h>
#include <flawless/timer/swTimer.h>
#include <flawless/platform/system.h>

#include <flawless/protocol/genericFlawLessProtocolApplication.h>
#include <flawless/protocol/genericFlawLessProtocol_Data.h>

#include <string.h>
#include <math.h>

#include <interfaces/IMU/IMU.h>
#include <interfaces/gyroData.h>
#include <interfaces/configuration.h>
#include <interfaces/systemTime.h>
#include <interfaces/reset.h>

#include <math/quaternion.h>


#include <interfaces/i2cCommunication.h>
#include "target/stm32f4/p2/IMU_phy/L3GD20.h"
#include "target/stm32f4/p2/IMU_phy/LSM303.h"

#include "imu/kalmanFilter.h"
#include "imu/gyroCalibration.h"
#include "imu/offsetCalibration.h"

MSG_PUMP_DECLARE_MESSAGE_BUFFER_POOL(imuSendData, uint8_t, 1, MSG_ID_IMU_SEND_DATA)


//-------------------------------------------------------
// Declaring struct and variables for saving calibration data

/** Saves the mean of the gyroscope
 * */
typedef struct _calibrationGyroscope_t {
	float mean[3];
} calibrationGyroscope_t;

/** Saves the min and max values of the magnetometer
 * */
typedef struct _calibrationMagnetometer_t {
	float min[3];
	float max[3];
} calibrationMagnetometer_t;

/** Saves the offset by which the gyro is rotated
 * The gyro values will be rotated back by these values
 * before sent to odroid
 */
typedef struct _calibrationQuaternion_t {
	float offsetRotation[4];
} calibrationQuaternion_t;

/** All IMU calibration data together
 * */
typedef struct _calibrationIMU_t {
	calibrationGyroscope_t gyro;
	calibrationMagnetometer_t mag;
	calibrationQuaternion_t quat;
	float Q;
	float R;
} calibrationIMU_t;

// Data in g_calibrationIMU is loaded from flash (given initialization values have no meaning)
static const calibrationIMU_t calibrationIMUDefaultValues = {{{0.0f, 0.0f, 0.0f}},
        {{-0.03718f, 0.41420f, 0.0f}, {0.273500f, 0.832438f, 0.0f}},
        {{1.0f, 0.0f, 0.0f, 0.0f}},
		0.001f, 1.0f};

CONFIG_VARIABLE(calibrationIMU_t, g_calibrationIMU, &calibrationIMUDefaultValues)

/* The IMU is not the right way in the robot
 * The axis are being swaped
 */
static void swapAxis(float* v)
{
	float t = v[1];
	v[0] = -v[0];
	v[1] = -v[2];
	v[2] = t;
}


// Current valid IMU data
static imuData_t g_imuAccData;
static imuData_t g_imuGyroData;
static imuData_t g_imuMagData;

// Last IMU data (for detecting freeze
static imuData_t g_imuAccDataOld;
static uint32_t g_imuAccDataOldCt;
static imuData_t g_imuGyroDataOld;
static uint32_t g_imuGyroDataOldCt;


// Current hz of updates
static float g_hz[3];

// Timestamp of last valid imu data
static uint64_t timestampUS = 0;

// Timestamp in micro seconds of last gyrodata
static uint64_t lastGyroUpdateInUS = 0;
static uint64_t lastAccelUpdateInUS = 0;
static uint64_t lastMagUpdateInUS = 0;


// Calibration mode of gyro and mag (0: off, 1: on)
static uint8_t g_calibrationMagMode = 0;

// identicator if magnetometer should be applied
static uint8_t g_usemagnetometer = 0;
static float g_currentYaw = 0.0f;

/** Message structur that is send from erolf-p2 to odroid
 */
typedef struct _imuOutgoingMsg_t
{
	uint64_t timestampUS;
	float ekf_quaternion[4]; // ekf filtered orientation as a quaternion
	float ekf_euler[3];      // ekf filtered Orientation in euler angles, in radians
	float rawAccel[3];       // unfiltered accelerometerdata
	float rawGyro[3];        // unfiltered gyroscope data
	float rawMag[3];         // raw magnetometer values in gauss
	uint16_t sensorHz[3];    // current updates of sensor per second, 0 accel, 1 gyro, 2 magnetometer
	uint16_t flags; /* flags
						0: accel not working
						1: gyro not working
					*/
} imuOutgoingMsg_t;


/** Message structur that is send from odroid to erolf-p2
 * mode:
 * 0: start gyro calibration
 * 1: stop gyro calibration
 * 2: start mag calibration
 * 3: stop mag calibration
 * 4: start offset calibration
 * 5: stop offset calibration
 * 6: send new ekf parameters
 * 10: sending oppgoal update
 */
typedef struct _imuIncomingMsg_t
{
	uint32_t mode;
	float Q;
	float R;
	float oppGoalAngle;
} imuIncomingMsg_t;



//-------------------------------------------------
/* Functions to compute and apply gyro magnetometer
 *
 * To calibrate magnetometer, we need to know
 * maximun and minimun of magnet field in any orientation.
 * For simplicity we only calibrate and apply values 2D plane
 * and apply the data in a way, that we only get information about
 * the yaw.
 *
 */

/* activate magnetometer calibration mode
 *
 */
static void startMagCalibration()
{
	g_calibrationMagMode = 1;
}


/* Applying calibration and/or computing new calibrating on magnetometer data
 *
 */
static void runMagCalibration()
{
	int8_t i;
	float* min = g_calibrationIMU.mag.min;
	float* max = g_calibrationIMU.mag.max;

	if (g_calibrationMagMode == 1)
	{
		for (i = 0; i<3; ++i)
			min[i] = max[i] = g_imuMagData.v[i];

		g_calibrationMagMode = 2;
	}
	else if(g_calibrationMagMode == 2)
	{
		for (i = 0;i < 3; ++i) {
			if (g_imuMagData.v[i] < min[i]) {
				min[i] = g_imuMagData.v[i];
			}else if (g_imuMagData.v[i] > max[i]) {
				max[i] = g_imuMagData.v[i];
			}
		}
	}

	g_imuMagData.v[0] -= (max[0]+min[0]) * 0.5f;
	g_imuMagData.v[1] -= (max[1]+min[1]) * 0.5f;

	if ((max[0]-min[0]) > 0.1f && (max[1]-min[1]) > 0.1f)
	{
		g_imuMagData.v[0] /= (max[0]-min[0]) * 0.5f;
		g_imuMagData.v[1] /= (max[1]-min[1]) * 0.5f;
	}
}
/* stop magnetometer calibrating
 *
 */
static void stopMagCalibration()
{
	g_calibrationMagMode = 0;
}

//--------------------------------------------------------------------
// Function for sending data from erolf-p2 to odroid

/* timer callback for posting a msg, that new gyrodata should be send to odroid
 *
 */
static void imu_timerCB(void)
{
	int x=0;
	msgPump_postMessage(MSG_ID_IMU_SEND_DATA, &x);
}

static void imu_restartP2(void)
{
	system_reset();
}

static void app_imu_send_msgPumpCallback(msgPump_MsgID_t id, const void *buffer)
{
	UNUSED(id);
	UNUSED(buffer);

	runOffsetCalibration(g_calibrationIMU.quat.offsetRotation);

	imuOutgoingMsg_t trans;
	float const* _q = kf_getState();

	// Fill pitch/yaw/roll data and quaternion data
	QUAT_INIT_DED_BUF(rotationOffset, g_calibrationIMU.quat.offsetRotation);
	QUAT_INIT_SET(quat,_q[0],_q[1],_q[2],_q[3]);
	quat_mul(&quat,&quat,&rotationOffset);
	float* q = quat.vector4d->data;

	int i;
	for(i = 0; i < 4; ++i)
		trans.ekf_quaternion[i] = q[i];
	{
		float t = q[1];
		q[1] = q[3];
		q[3] = q[2];
		q[2] = t;

		float sqw = q[0]*q[0];
		float sqx = q[1]*q[1];
		float sqy = q[2]*q[2];
		float sqz = q[3]*q[3];
		//roll, pitch and yaw
		trans.ekf_euler[0] = -asinf(-2.0f * (q[1]*q[3] - q[2]*q[0])/(sqx + sqy + sqz + sqw));
		trans.ekf_euler[1] = atan2f(2.0f * (q[1]*q[2] + q[3]*q[0]),(sqx - sqy - sqz + sqw));
		trans.ekf_euler[2] = atan2f(2.0f * (q[2]*q[3] + q[1]*q[0]),(-sqx - sqy + sqz + sqw));

		g_currentYaw = trans.ekf_euler[2];

	}

	for(i =0;i < 3; ++i)
		trans.sensorHz[i] = g_hz[i];


	//Magnetometer data
	g_usemagnetometer = (fabsf(trans.ekf_euler[0]) < (10.0f/180.0f*3.141598f)
						&& fabsf(trans.ekf_euler[1]) < (10.0f/180.0f*3.141598f))?1:0;

	// copy raw accel, gyro and mag data
	memcpy(trans.rawAccel, g_imuAccData.v,  sizeof(g_imuAccData.v));
	memcpy(trans.rawGyro,  g_imuGyroData.v, sizeof(g_imuGyroData.v));
	memcpy(trans.rawMag,   g_imuMagData.v,  sizeof(g_imuMagData.v));

	// Timestamp
	trans.timestampUS = convertToSyncUS(timestampUS);

	// Setflags
	trans.flags = 0; //Clear all

	if (g_imuAccDataOldCt > 100) {
		trans.flags |= 1<<0;
	} else if (g_imuGyroDataOldCt > 100) {
		trans.flags |= 1<<1;
	}

	if (trans.flags != 0) {
		swTimer_registerOnTimer(&imu_restartP2, 100, FALSE);

	}


	genericProtocol_sendMessage(0, 11, sizeof(trans), &trans);
}

/** checks if data from flash need to be initialized
 *
 * By checking for not valid numbers, we can discover
 * if the data was previously saved into flash
 * or if we have to initialize with default values
 */
static void loadDefaultValues(void)
{
	int loadDefault = 0;
	int i;
	for (i = 0; i < 3; ++i) {
		loadDefault |= isnan(g_calibrationIMU.gyro.mean[i]);
		loadDefault |= isnan(g_calibrationIMU.mag.min[i]);
		loadDefault |= isnan(g_calibrationIMU.mag.max[i]);
		loadDefault |= isnan(g_calibrationIMU.quat.offsetRotation[i]);
	}
	loadDefault |= isnan(g_calibrationIMU.quat.offsetRotation[3]);
	loadDefault |= isnan(g_calibrationIMU.Q);
	loadDefault |= isnan(g_calibrationIMU.R);

	if (loadDefault)
	{
		calibrationIMU_t defaultCalibrationIMU = {{{0.0f, 0.0f, 0.0f}},
        {{-0.03718f, 0.41420f, 0.0f}, {0.273500f, 0.832438f, 0.0f}},
        {{1.0f, 0.0f, 0.0f, 0.0f}},
		0.001f, 1.0f};

		memcpy(&g_calibrationIMU, &defaultCalibrationIMU, sizeof(calibrationIMU_t));
	}
	g_imuAccDataOldCt = 0;
	g_imuGyroDataOldCt = 0;
}

//------------------------------------------
// Function for Receiving data from gyro

/** Receives data from accel and hands it to the kalmanfilter
 */
static void app_imu_accel_msgPumpCallback(msgPump_MsgID_t id, const void *buffer)
{
	UNUSED(id);

	memcpy(&g_imuAccData, buffer, sizeof(g_imuAccData));
	swapAxis(g_imuAccData.v);

	// Detect gyro freeze
	if (g_imuAccDataOld.v[0] == g_imuAccData.v[0]
	    && g_imuAccDataOld.v[1] == g_imuAccData.v[1]
	    && g_imuAccDataOld.v[2] == g_imuAccData.v[2]) {
		++g_imuAccDataOldCt;
	} else {
		g_imuAccDataOldCt = 0;
	}
	memcpy(&g_imuAccDataOld, &g_imuAccData, sizeof(g_imuAccData));


	uint64_t diff = g_imuAccData.timestampUS - lastAccelUpdateInUS;
	if (diff != 0ULL)
		g_hz[0] = 1000000.0f / ((float)diff);
	else
		g_hz[0] = 0;

	timestampUS = g_imuAccData.timestampUS;
	lastAccelUpdateInUS = g_imuAccData.timestampUS;

	kf_update_accel(g_imuAccData.v);
}

/** Receives data from gyro and hands it to the kalmanfilter
 */
static void app_imu_gyro_msgPumpCallback(msgPump_MsgID_t id, const void *buffer)
{
	UNUSED(id);

	memcpy(&g_imuGyroData, buffer, sizeof(g_imuGyroData));
	swapAxis(g_imuGyroData.v);

	// Convert data to radians
	g_imuGyroData.v[0] *=  (float)M_PI / 180.0f;
	g_imuGyroData.v[1] *=  (float)M_PI / 180.0f;
	g_imuGyroData.v[2] *=  (float)M_PI / 180.0f;

	// Detect gyro freeze
	if (g_imuGyroDataOld.v[0] == g_imuGyroData.v[0]
	    && g_imuGyroDataOld.v[1] == g_imuGyroData.v[1]
	    && g_imuGyroDataOld.v[2] == g_imuGyroData.v[2]) {
		++g_imuGyroDataOldCt;
	} else {
		g_imuGyroDataOldCt = 0;
	}
	memcpy(&g_imuGyroDataOld, &g_imuGyroData, sizeof(g_imuGyroData));
	

	timestampUS = g_imuGyroData.timestampUS;
	runGyroCalibration(g_calibrationIMU.gyro.mean, g_imuGyroData.timestampUS, g_imuGyroData.v);

	float timepassed = ((float)(timestampUS - lastGyroUpdateInUS)) / 1000000.0f;

	kf_predict(g_imuGyroData.v, timepassed);

	uint64_t diff = g_imuGyroData.timestampUS - lastGyroUpdateInUS;
	if (diff != 0ULL)
		g_hz[1] = 1000000.0f / ((float)diff);
	else
		g_hz[1] = 0;

	lastGyroUpdateInUS = timestampUS;

}

/** Receives data from magnetometer and hands it to the kalmanfilter
 */
static void app_imu_mag_msgPumpCallback(msgPump_MsgID_t id, const void *buffer) {

	UNUSED(id);

	memcpy(&g_imuMagData, buffer, sizeof(g_imuMagData));
	swapAxis(g_imuMagData.v);

	uint64_t diff = g_imuMagData.timestampUS - lastMagUpdateInUS;
	if (diff != 0ULL)
		g_hz[2] = 1000000.0f / ((float)diff);
	else
		g_hz[2] = 0;

	lastMagUpdateInUS = g_imuMagData.timestampUS;


//	if(1 == g_usemagnetometer)
	{
		timestampUS = g_imuMagData.timestampUS;
//		float norm = (g_imuMagData.v[1] - (g_calibrationIMU.mag.max[1] + g_calibrationIMU.mag.min[1]) * 0.5f); //adjust zero
//		float d = (g_calibrationIMU.mag.max[1] - g_calibrationIMU.mag.min[1]) * 0.5f;
//		if (d > 0.0001f) // check that no division by zero occures
		{
//			norm = norm / d; //ajust scale
//			g_imuMagData.v[1] = norm;
			runMagCalibration();

			float norm = g_imuMagData.v[1];

			float magYaw = asinf(norm);
			if (fabsf(magYaw - g_currentYaw) > fabsf(M_PI - magYaw - g_currentYaw)) {
				magYaw = M_PI - magYaw;
			}
				float mag[3];
			mag[0] = cos(magYaw);
			mag[1] = sin(magYaw);
			mag[2] = 0;

			UNUSED(mag);
//			kf_update_mag(mag);
		}
	}
}


/** Initializes gyroReader
 */
static void imu_init(void);
MODULE_INIT_FUNCTION(imu, 9, imu_init);
static void imu_init(void)
{
	loadDefaultValues();
	swTimer_registerOnTimer(&imu_timerCB, 5, FALSE);


	const bool registerSuccess1 = msgPump_registerOnMessage(MSG_ID_ACCEL_SCALED_DATA,  &app_imu_accel_msgPumpCallback);
	ASSERT(FALSE != registerSuccess1);

	const bool registerSuccess2 = msgPump_registerOnMessage(MSG_ID_GYRO_SCALED_DATA,  &app_imu_gyro_msgPumpCallback);
	ASSERT(FALSE != registerSuccess2);

	const bool registerSuccess3 = msgPump_registerOnMessage(MSG_ID_MAGNETO_SCALED_DATA,  &app_imu_mag_msgPumpCallback);
	ASSERT(FALSE != registerSuccess3);

	const bool registerSuccess4 = msgPump_registerOnMessage(MSG_ID_IMU_SEND_DATA,  &app_imu_send_msgPumpCallback);
	ASSERT(FALSE != registerSuccess4);

	lastGyroUpdateInUS = getSystemTimeUS();

	kf_init();
	kf_set_parameter(g_calibrationIMU.Q, g_calibrationIMU.R);
}

/** receives incoming messages from odroid
 */
static void gyroCtlEndpoint(const void *ipacket, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor);
GENERIC_PROTOCOL_ENDPOINT(gyroCtlEndpoint, 11);
static void gyroCtlEndpoint(const void *ipacket, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor)
{
	UNUSED(i_interfaceDescriptor);
	ASSERT(i_packetLen == sizeof(imuIncomingMsg_t));
	imuIncomingMsg_t incomingMsg;
	memcpy(&incomingMsg, ipacket, sizeof(imuIncomingMsg_t));

	if (incomingMsg.mode == 0)
	{
		startGyroCalibration(g_calibrationIMU.gyro.mean);
	}
	else if (incomingMsg.mode == 1)
	{
		stopGyroCalibration(g_calibrationIMU.gyro.mean);
	}
	else if (incomingMsg.mode == 2)
	{
		startMagCalibration();
	}
	else if (incomingMsg.mode == 3)
	{
		stopMagCalibration();
	}
	else if (incomingMsg.mode == 4)
	{
		startOffsetCalibration(g_calibrationIMU.quat.offsetRotation);
	}
	else if (incomingMsg.mode == 5)
	{
		stopOffsetCalibration(g_calibrationIMU.quat.offsetRotation);
	}
	else if (incomingMsg.mode == 6)
	{
		g_calibrationIMU.Q = incomingMsg.Q;
		g_calibrationIMU.R = incomingMsg.R;
		kf_set_parameter(g_calibrationIMU.Q, g_calibrationIMU.R);
		config_updateToFlash();
	}
	else if (incomingMsg.mode == 10)
	{
		float yawOffset = 0.0f;
		{ //offset yaw?
			float const* f = g_calibrationIMU.quat.offsetRotation;
			QUAT_INIT_SET(quat,f[0],f[1],f[2],f[3]);
			float* q = quat.vector4d->data;

			float t = q[1];
			q[1] = q[3];
			q[3] = q[2];
			q[2] = t;

			float sqw = q[0]*q[0];
			float sqx = q[1]*q[1];
			float sqy = q[2]*q[2];
			float sqz = q[3]*q[3];

	  		yawOffset = atan2f(2.0f * (q[2]*q[3] + q[1]*q[0]),(-sqx - sqy + sqz + sqw));
		}

		float mag[3];
		float alpha = incomingMsg.oppGoalAngle - yawOffset;
		mag[0] = -cosf(alpha);
		mag[1] = -sinf(alpha);
		mag[2] = 0.0f;
		kf_update_mag(mag);
	}
}



