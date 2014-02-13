/*
 * idleMotion.c
 *
 *  Created on: Mar 30, 2013
 *      Author: lutz
 */


#include <flawless/init/systemInitializer.h>

#include <flawless/core/msg_msgPump.h>
#include <flawless/config/msgIDs.h>
#include <flawless/rpc/RPC.h>

#include <interfaces/usrButtons.h>
#include <interfaces/motorIDs.h>
#include <interfaces/dynamixel.h>
#include <interfaces/configuration.h>

#include <string.h>

#define IDLE_ADDITIONAL_ARM_OFFSET 60

typedef uint16_t motorValuesAllMotors_t[MOTOR_LAST_MOTOR];
static const motorValuesAllMotors_t motorDefaultValues = {
		512,
		512,
		512,
		512,
		512,
		512,
		512,
		512,
		512,
		512,
		512,
		512,
		512,
		512,
		512,
		512,
		512,
		512,
		512,
		512
};

CONFIG_VARIABLE(motorValuesAllMotors_t, g_idleValues, &motorDefaultValues);

static const uint16_t g_motorIdleSpeeds[MOTOR_LAST_MOTOR] =
{
		100,
		100,
		100, /* left arm roll */
		100, /* right arm roll */
		100, /* left arm pitch */
		100, /* right arm pitch */
		100, /* left elbow */
		100, /* right elbow */
		200,
		200,
		200,
		200,
		200,
		200,
		200,
		200,
		200,
		200,
		200,
		200
};

static const motorID_t g_motorIDs[MOTOR_LAST_MOTOR] =
{
		1,
		2,
		3,
		4,
		5,
		6,
		7,
		8,
		9,
		10,
		11,
		12,
		13,
		14,
		15,
		16,
		17,
		18,
		19,
		20
};

typedef uint16_t motorValues[MOTOR_LAST_MOTOR];

RPC_FUNCTION(setIdleValues, "setIdleValues", "set motor Positions for idle motion", motorValues, uint8_t, false, true)
	const uint16_t *values = (const uint16_t*)i_param;
	uint8_t i = 0U;
	for (i = 0U; i < MOTOR_LAST_MOTOR; ++i)
	{
		const uint16_t value = (values[i] > 1023) ? 512 : values[i];
		g_idleValues[i] = value;
	}
	config_updateToFlash();
}

RPC_FUNCTION(getIdleValues, "getIdleValues", "set motor Positions for idle motion", uint8_t, motorValues, true, false)
	memcpy(o_param, g_idleValues, sizeof(g_idleValues));
}


bool addOffsets(const motorID_t *ids, uint8_t cnt, uint16_t **values)
{
	bool ret = false;
	if (NULL != ids && NULL != values)
	{
		uint8_t i = 0U;
		for (i = 0; i < cnt; ++i)
		{
			motorID_t curMotor = ids[i];
			uint16_t *valuePtr = values[i];
			if ((NULL != valuePtr) &&
				(curMotor >= MOTOR_FIRST_MOTOR) &&
				(curMotor <= MOTOR_LAST_MOTOR))
			{
				int16_t offset = g_idleValues[curMotor - MOTOR_FIRST_MOTOR] - 512;
				*valuePtr += offset;
			}
		}
		ret = true;
	}
	return ret;
}


bool removeOffsets(const motorID_t *ids, uint8_t cnt, uint16_t **values)
{
	bool ret = false;
	if (NULL != ids && NULL != values)
	{
		uint8_t i = 0U;
		for (i = 0; i < cnt; ++i)
		{
			motorID_t curMotor = ids[i];
			uint16_t *valuePtr = values[i];
			if ((NULL != valuePtr) &&
				(curMotor >= MOTOR_FIRST_MOTOR) &&
				(curMotor <= MOTOR_LAST_MOTOR))
			{
				int16_t offset = g_idleValues[curMotor - MOTOR_FIRST_MOTOR] - 512;
				*valuePtr -= offset;
			}
		}
		ret = true;
	}
	return ret;
}

static void performIdlePose()
{
	uint8_t i = 0U;
	const uint16_t *data[MOTOR_LAST_MOTOR];
	motorValuesAllMotors_t idlePose;
	memcpy(&idlePose, &g_idleValues, sizeof(idlePose));
	idlePose[MOTOR_LEFT_ARM_ROLL - MOTOR_FIRST_MOTOR] -= IDLE_ADDITIONAL_ARM_OFFSET;
	idlePose[MOTOR_RIGHT_ARM_ROLL - MOTOR_FIRST_MOTOR] += IDLE_ADDITIONAL_ARM_OFFSET;

	/* first set the speeds: */
	for (i = 0U; i < MOTOR_LAST_MOTOR; ++i)
	{
		data[i] = &(g_motorIdleSpeeds[i]);
	}
	dynamixel_sync_write(g_motorIDs, MOTOR_LAST_MOTOR, DYNAMIXEL_REGISTER_MOVING_SPEED, (const uint8_t**)data, sizeof(uint16_t), DYNAMIXEL_PORT_ALL, DYNAMIXEL_TRANSACTION_PRIORITY_HIGH);

	/* aaaand the target position */
	for (i = 0U; i < MOTOR_LAST_MOTOR; ++i)
	{
		data[i] = &(idlePose[i]);
	}
	dynamixel_sync_write(g_motorIDs, sizeof(g_motorIDs), DYNAMIXEL_REGISTER_GOAL_POSITION, (const uint8_t**)data, sizeof(uint16_t), DYNAMIXEL_PORT_ALL, DYNAMIXEL_TRANSACTION_PRIORITY_HIGH);
}


static void onButtonPressed(msgPump_MsgID_t msgID, const void* data);
static void onButtonPressed(msgPump_MsgID_t msgID, const void* data)
{
	if (MSG_ID_USR_BUTTON_1_PRESSED == msgID)
	{
		usrButtonState_t state = *((const usrButtonState_t*) data);
		if (USR_BUTTON_PRESSED == state)
		{
			performIdlePose();
		}
	}
}


static void idleMotion_init(void);
MODULE_INIT_FUNCTION(idleMotion, 9, idleMotion_init)
static void idleMotion_init(void)
{
	msgPump_registerOnMessage(MSG_ID_USR_BUTTON_1_PRESSED, &onButtonPressed);
	setupButton(USR_BUTTON_1, USR_BUTTON_CNF_LED_ON_LISTENING);
}


