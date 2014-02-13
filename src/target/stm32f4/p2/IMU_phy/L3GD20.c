/*
 * L3GD20.c
 *
 *  Created on: Nov 28, 2012
 *      Author: lutz
 */


#include <flawless/init/systemInitializer.h>
#include <flawless/timer/swTimer.h>

#include <flawless/core/msg_msgPump.h>
#include <flawless/config/msgIDs.h>

#include "L3GD20.h"

#include <interfaces/i2cCommunication.h>
#include <interfaces/gyroData.h>
#include <interfaces/systemTime.h>

#define L3GD20_SCALE ( 8.75f / 1000.f)
//#define L3GD20_SCALE ( 70.f / 1000.f)

MSG_PUMP_DECLARE_MESSAGE_BUFFER_POOL(gyroRaw, l3gd20_gyroDataRaw_t, 2, MSG_ID_GYRO_RAW_DATA)
MSG_PUMP_DECLARE_MESSAGE_BUFFER_POOL(gyroProc, imuData_t, 2, MSG_ID_GYRO_SCALED_DATA)


static void onNewRawGyroData(msgPump_MsgID_t id, const void *buffer);
static void onNewRawGyroData(msgPump_MsgID_t id, const void *buffer)
{
	if ((MSG_ID_GYRO_RAW_DATA == id) &&
		(NULL != buffer))
	{
		/* do post processing */
		const l3gd20_gyroDataRaw_t *data = (const l3gd20_gyroDataRaw_t*) buffer;
		imuData_t processed;

		processed.timestampUS = getSystemTimeUS();
		processed.v[0] = ((float)(*data)[0]) * L3GD20_SCALE;
		processed.v[1] = ((float)(*data)[1]) * L3GD20_SCALE;
		processed.v[2] = ((float)(*data)[2]) * L3GD20_SCALE;

		msgPump_postMessage(MSG_ID_GYRO_SCALED_DATA, &processed);
	}
}

static void gyroConfigDataCallback(uint8_t *receivedData,
					uint8_t dataCnt,
					i2c_slaveAddress_t device,
					i2c_register_t subReg);
static void gyroConfigDataCallback(uint8_t *receivedData,
					uint8_t dataCnt,
					i2c_slaveAddress_t device,
					i2c_register_t subReg)
{
	UNUSED(receivedData);
	UNUSED(dataCnt);
	UNUSED(device);
	UNUSED(subReg);
}




static l3gd20_gyroDataRaw_t g_gyroData;
static void gyroDataCallback(uint8_t *receivedData,
					uint8_t dataCnt,
					i2c_slaveAddress_t device,
					i2c_register_t subReg);
static void gyroDataCallback(uint8_t *receivedData,
					uint8_t dataCnt,
					i2c_slaveAddress_t device,
					i2c_register_t subReg)
{
	UNUSED(receivedData);
	UNUSED(dataCnt);
	UNUSED(device);
	UNUSED(subReg);
	if ((sizeof(l3gd20_gyroDataRaw_t) == dataCnt) &&
		(NULL != receivedData) &&
		(L3GD20_DEVICE_ADRESS == device) &&
		(L3GD20_REGISTER_OUT_X_L == subReg))
	{
		// post the data
		msgPump_postMessage(MSG_ID_GYRO_RAW_DATA, g_gyroData);
	}
}

static void pollData(void);
static void pollData(void)
{
	bool succ = i2c_transact(L3GD20_DEVICE_ADRESS, L3GD20_REGISTER_OUT_X_L, NULL, 0, (uint8_t*)g_gyroData, sizeof(g_gyroData), &gyroDataCallback);
	static uint8_t ct = 0;
	if(FALSE == succ) {
		++ct;
		if(ct > 10) {
			i2cSensorStickForceReset();
			L3GD20_setupSensor();
			ct = 0;
		}

	} else ct = 0;
}

void L3GD20_setupSensor(void)
{
	/* setup the gyro by letting it enter the normal mode (PD bit set) */
	static uint8_t txBuf[5];
	static uint8_t rxBuf[5];
	txBuf[0] = 0xcf; /* CTRL_REG1 with 760Hz sample rate PD and Xen, Yen, Zen */
	txBuf[1] = 0x09; /* CTRL_REG2 */
	txBuf[2] = 0x00; /* CTRL_REG3 */
	txBuf[3] = 0x00; /* CTRL_REG4 without BLE for little endian transmission and 250dps scale */
	txBuf[4] = 0x00; /* CTRL_REG5 */

	i2c_transact(L3GD20_DEVICE_ADRESS, L3GD20_REGISTER_CTRL_REG1, txBuf, sizeof(txBuf), NULL, 0, NULL);
	i2c_transact(L3GD20_DEVICE_ADRESS, L3GD20_REGISTER_CTRL_REG1, NULL, 0, rxBuf, sizeof(rxBuf), &gyroConfigDataCallback);
}

static void L3GD20_firstInitSensor(void);
static void L3GD20_firstInitSensor(void)
{
	L3GD20_setupSensor();
	// Its running with 760 Hz, so we update every 1.315ms
	//swTimer_registerOnTimerUS(&pollData, 1315U*10U, false);
	swTimer_registerOnTimerUS(&pollData, 1315U, false);

	//swTimer_registerOnTimer(&pollData, 10U, false);

}

static void L3GD20_init(void);
MODULE_INIT_FUNCTION(L3GD20, 8, L3GD20_init)
static void L3GD20_init(void)
{
	swTimer_registerOnTimer(&L3GD20_firstInitSensor, 1000, true);
	//L3GD20_firstInitSensor();

	msgPump_registerOnMessage(MSG_ID_GYRO_RAW_DATA, &onNewRawGyroData);
}


