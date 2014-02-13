
#include "offsetCalibration.h"

#include "kalmanFilter.h"

#include "math/quaternion.h"
#include <interfaces/configuration.h>


// Calibration mode of gyro and mag (0: off, 1: on)
static uint8_t g_calibrationOffsetMode = 0;

// helper variable for calibrating offset
static uint16_t g_offsetCt = 0;



//------------------------------------------------
/** Functions to calibrate offsets of robot
 */
void startOffsetCalibration(float* offsetQuaternion)
{
	if (g_calibrationOffsetMode == 0) {
		g_calibrationOffsetMode = 1;
		g_offsetCt = 0;

		uint8_t i;
		offsetQuaternion[0] = -kf_getState()[0];
		for (i=1; i<4; ++i)
			offsetQuaternion[i] = kf_getState()[i];
	}

}
/** 
 * This will calculates an avarage of offset so that little jumps have no influence
 */
void runOffsetCalibration(float* offsetQuaternion)
{
	if (g_calibrationOffsetMode == 1)
	{
		float offsetCt = g_offsetCt;

		float const* f = kf_getState();
		float* r       = offsetQuaternion;

		r[0] = (r[0]*offsetCt - f[0]) / (offsetCt+1.0f);
		uint8_t i;
		for (i = 1; i<4; ++i)
			r[i] = (r[i]*offsetCt + f[i]) / (offsetCt+1.0f);

		g_offsetCt++;

		// normalize
		QUAT_INIT_DED_BUF(quat_r, r);
		vec_normalize(quat_r.vector4d, quat_r.vector4d);
	}

}
/**
 * Since we assume that the robot gets calibratet on its stomach we have to turn it 90 degree around
 * it  y axis
 */
void stopOffsetCalibration(float* offsetQuaternion)
{
	// Robots get calibrated on stomach, so needs to be turned 90degree back
	if (g_calibrationOffsetMode == 1)
	{
		QUAT_INIT_SET(rot, 0.70711, -0.70711, 0, 0); // This is 90 degree around its y axis
		QUAT_INIT_DED_BUF(quat_r, offsetQuaternion);

		quat_mul(&quat_r, &quat_r, &rot);

		g_calibrationOffsetMode = 0;

		config_updateToFlash();
	}
}
