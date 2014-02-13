/*
 * LSM303.c
 *
 *  Created on: Nov 30, 2012
 *      Author: lutz
 */


#include "LSM303.h"

#include <flawless/init/systemInitializer.h>
#include <flawless/timer/swTimer.h>

#include <flawless/misc/communication.h>

#include <flawless/core/msg_msgPump.h>
#include <flawless/config/msgIDs.h>

#include <interfaces/i2cCommunication.h>
#include <interfaces/gyroData.h>
#include <interfaces/systemTime.h>

#define LSM303_ACC_SCALE (4.f / ((float) 0x7fff));
#define LSM303_MAG_SCALE (4.f / (float) 0x07ff);

MSG_PUMP_DECLARE_MESSAGE_BUFFER_POOL(magRaw, lsm303_magDataRaw_t, 2, MSG_ID_MAGNETO_RAW_DATA)
MSG_PUMP_DECLARE_MESSAGE_BUFFER_POOL(magScaled, imuData_t, 2, MSG_ID_MAGNETO_SCALED_DATA)

MSG_PUMP_DECLARE_MESSAGE_BUFFER_POOL(accRaw, lsm303_accDataRaw_t, 2, MSG_ID_ACCEL_RAW_DATA)
MSG_PUMP_DECLARE_MESSAGE_BUFFER_POOL(accScaled, imuData_t, 2, MSG_ID_ACCEL_SCALED_DATA)


static void onNewRawMagData(msgPump_MsgID_t id, const void *buffer);
static void onNewRawMagData(msgPump_MsgID_t id, const void *buffer)
{
	if ((MSG_ID_MAGNETO_RAW_DATA == id) &&
		(NULL != buffer))
	{
		/* do post processing */
		const lsm303_magDataRaw_t *data = (const lsm303_magDataRaw_t*) buffer;
		imuData_t processed;

		processed.timestampUS = getSystemTimeUS();
		processed.v[0] = ((float)((int16_t)HTONS((*data)[0]))) * LSM303_MAG_SCALE;
		processed.v[1] = ((float)((int16_t)HTONS((*data)[1]))) * LSM303_MAG_SCALE;
		processed.v[2] = ((float)((int16_t)HTONS((*data)[2]))) * LSM303_MAG_SCALE;

		msgPump_postMessage(MSG_ID_MAGNETO_SCALED_DATA, &processed);
	}
}

static void onNewRawAccData(msgPump_MsgID_t id, const void *buffer);
static void onNewRawAccData(msgPump_MsgID_t id, const void *buffer)
{
	if ((MSG_ID_ACCEL_RAW_DATA == id) &&
		(NULL != buffer))
	{
		/* do post processing */
		const lsm303_accDataRaw_t *data = (const lsm303_accDataRaw_t*) buffer;
		imuData_t processed;

		processed.timestampUS = getSystemTimeUS();
		processed.v[0] = ((float)((*data)[0])) * LSM303_ACC_SCALE;
		processed.v[1] = ((float)((*data)[1])) * LSM303_ACC_SCALE;
		processed.v[2] = ((float)((*data)[2])) * LSM303_ACC_SCALE;

		msgPump_postMessage(MSG_ID_ACCEL_SCALED_DATA, &processed);
	}
}

static void configDataCallback(uint8_t *receivedData,
					uint8_t dataCnt,
					i2c_slaveAddress_t device,
					i2c_register_t subReg);
static void configDataCallback(uint8_t *receivedData,
					uint8_t dataCnt,
					i2c_slaveAddress_t device,
					i2c_register_t subReg)
{
	UNUSED(receivedData);
	UNUSED(dataCnt);
	UNUSED(device);
	UNUSED(subReg);
}



static lsm303_accDataRaw_t g_accData;
static void accDataCallback(uint8_t *receivedData,
					uint8_t dataCnt,
					i2c_slaveAddress_t device,
					i2c_register_t subReg)
{
	if ((sizeof(lsm303_accDataRaw_t) == dataCnt) &&
		(NULL != receivedData) &&
		(LSM303_ACC_DEVICE_ADDRESS == device) &&
		(LSM303_ACC_OUT_X_L == subReg))
	{
		/* post the data */
		msgPump_postMessage(MSG_ID_ACCEL_RAW_DATA, g_accData);
	}
}

static lsm303_magDataRaw_t g_magData;
static void magDataCallback(uint8_t *receivedData,
					uint8_t dataCnt,
					i2c_slaveAddress_t device,
					i2c_register_t subReg)
{
	if ((sizeof(lsm303_magDataRaw_t) == dataCnt) &&
		(NULL != receivedData) &&
		(LSM303_MAG_DEVICE_ADDRESS == device) &&
		(LSM303_MAG_OUT_X_H == subReg))
	{
		/* post the data */
		msgPump_postMessage(MSG_ID_MAGNETO_RAW_DATA, g_magData);
	}
}

static void pollAccData(void);
static void pollAccData(void)
{
	i2c_transact(LSM303_ACC_DEVICE_ADDRESS, LSM303_ACC_OUT_X_L, NULL, 0, (uint8_t*)g_accData, sizeof(g_accData), &accDataCallback);
}

static void pollMagData(void);
static void pollMagData(void)
{
	i2c_transact(LSM303_MAG_DEVICE_ADDRESS, LSM303_MAG_OUT_X_H, NULL, 0, (uint8_t*)g_magData, sizeof(g_magData), &magDataCallback);
}

void LSM303_setupSensor(void)
{
	/* setup the gyro by letting it enter the normal mode (PD bit set) */
	static uint8_t txAccBuf[6];
	static uint8_t txMagBuf[4];
	static uint8_t rxMagBuf[3] = {0};

	txAccBuf[0] = 0x3f; /* CTRL_REG1 with 100Hz update interval and all dimensions active */
	txAccBuf[1] = 0x00; /* CTRL_REG2 */
	txAccBuf[2] = 0x00; /* CTRL_REG3 */
	txAccBuf[3] = 0x10; /* CTRL_REG4 scale to 4G and high resolution enabled */
	txAccBuf[4] = 0x00; /* CTRL_REG5 */
	txAccBuf[5] = 0x00; /* dummy */

	txMagBuf[0] = 0x18; /* CTRL_REG1 with 75Hz update interval */
	txMagBuf[1] = 0x80; /* CTRL_REG2 with gain set to 4 Gauss input sensitivity */
	txMagBuf[2] = 0x00; /* CTRL_REG3 with continuous conversion mode */
	txMagBuf[3] = 0x00; /* dummy */

	i2c_transact(LSM303_ACC_DEVICE_ADDRESS, LSM303_ACC_REG_CTRL_REG1, txAccBuf, sizeof(txAccBuf), NULL, 0, NULL);
	i2c_transact(LSM303_MAG_DEVICE_ADDRESS, LSM303_MAG_CRA, txMagBuf, sizeof(txMagBuf), NULL, 0, NULL);
	i2c_transact(LSM303_MAG_DEVICE_ADDRESS, LSM303_MAG_CRA, NULL, 0, rxMagBuf, sizeof(rxMagBuf), &configDataCallback);
}
static void LSM303_firstInitSensor(void);
static void LSM303_firstInitSensor(void)
{
	LSM303_setupSensor();

	//Accelerometer is being updated very 100Hz, so we update every 10ms
	swTimer_registerOnTimerUS(&pollAccData, 10000, false);

	// Updates with 75Hz so we update every 13.333ms
	swTimer_registerOnTimerUS(&pollMagData, 13333U, false);

}
static void LSM303_init(void);
MODULE_INIT_FUNCTION(LSM303, 8, LSM303_init)
static void LSM303_init(void)
{
	swTimer_registerOnTimer(&LSM303_firstInitSensor, 1000, true);

	msgPump_registerOnMessage(MSG_ID_ACCEL_RAW_DATA, &onNewRawAccData);
	msgPump_registerOnMessage(MSG_ID_MAGNETO_RAW_DATA, &onNewRawMagData);
}

