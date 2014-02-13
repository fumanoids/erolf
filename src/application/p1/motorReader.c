/*
 * motors.c
 *
 *  Created on: Mar 7, 2013
 *      Author: lutz
 */

#include <flawless/init/systemInitializer.h>
#include <flawless/timer/swTimer.h>
#include <flawless/core/msg_msgPump.h>
#include <flawless/config/msgIDs.h>
#include <flawless/protocol/genericFlawLessProtocolApplication.h>
#include <flawless/protocol/genericFlawLessProtocol_Data.h>
#include <flawless/rpc/RPC.h>

#include <interfaces/dynamixel.h>
#include <interfaces/dynamixel_config.h>
#include <interfaces/motorIDs.h>
#include <interfaces/motorAssociations.h>

#include <interfaces/systemTime.h>
#include <interfaces/offsets.h>

#include <string.h>

#define COM_2_MAIN_COMPUTER_LOGIC_INTERFACE_NR 1U

MSG_PUMP_DECLARE_MESSAGE_BUFFER_POOL(triggerPushMotorValuesBP, int, 2, MSG_ID_TRIGGER_PUSH_MOTOR_VALUES)
MSG_PUMP_DECLARE_MESSAGE_BUFFER_POOL(triggerReadMotorValuesBP, int, 2, MSG_ID_TRIGGER_READ_MOTORS)

static const motorID_t torsoMotors[] = MOTORS_TORSO;
static const motorID_t leftLegMotors[] = MOTORS_LEFT_LEG;
static const motorID_t rightLegMotors[] = MOTORS_RIGHT_LEG;

#define MOTOR_READ_CYCLE_INTERVAL_MS 4U

#define MOTOR_DISTRIBUTE_INTERVAL_MS 10U


typedef struct tag_motorPosition
{
	int16_t position;
	int16_t speed;
	int16_t load;
} motorPosition_t;

typedef struct tag_motorInfo
{
	uint16_t successfullReads, missedReads;
	uint8_t error;
	uint8_t consecutiveFailedReads;
} motorInfo_t;

typedef int16_t motorPositionsArray_t[MOTOR_LAST_MOTOR];

static motorPosition_t g_motorPositions[MOTOR_LAST_MOTOR] = {{0, 0, 0}};
static motorPositionsArray_t g_motorPositionsNotAdjustedOffsets = {0};
static motorInfo_t g_motorInfos[MOTOR_LAST_MOTOR] = {{0, 0, 0, 0}};

static uint32_t g_totalSuccessfullReads = 0U;
static uint32_t g_totalUnsuccessfullReads = 0U;

#define MAX_CONSECUTIVE_READ_FAILURES 5U

static void onMotorRead(bool success,
						const uint8_t *receivedData,
						uint8_t receivedDataCnt,
						uint8_t error,
						dynamixel_motorID_t motor,
						dynamixel_instruction_t instruction)
{
	if (FALSE == success)
	{
		if ((motor <= MOTOR_LAST_MOTOR))
		{
			++g_totalUnsuccessfullReads;
			++(g_motorInfos[motor - MOTOR_FIRST_MOTOR].missedReads);
			if (g_motorInfos[motor - MOTOR_FIRST_MOTOR].consecutiveFailedReads >= (MAX_CONSECUTIVE_READ_FAILURES - 1U))
			{
				g_motorPositions[motor - MOTOR_FIRST_MOTOR].position = -1;
				g_motorPositionsNotAdjustedOffsets[motor - MOTOR_FIRST_MOTOR] = -1;
			} else
			{
				++(g_motorInfos[motor - MOTOR_FIRST_MOTOR].consecutiveFailedReads);
			}
		}
	} else
	{
		if ((motor <= MOTOR_LAST_MOTOR) &&
			(DYNAMIXEL_INSTRUCTION_READ == instruction) &&
			(sizeof(motorPosition_t) == receivedDataCnt))
		{
			const motorPosition_t *motorValues = (const motorPosition_t*)receivedData;
			++g_totalSuccessfullReads;
			++(g_motorInfos[motor - MOTOR_FIRST_MOTOR].successfullReads);
			g_motorInfos[motor - MOTOR_FIRST_MOTOR].error = error;
			g_motorInfos[motor - MOTOR_FIRST_MOTOR].consecutiveFailedReads = 0;

			g_motorPositions[motor - MOTOR_FIRST_MOTOR].position = motorValues->position;
			g_motorPositionsNotAdjustedOffsets[motor - MOTOR_FIRST_MOTOR] = motorValues->position;

			uint16_t *positionPtr = (uint16_t*)&(g_motorPositions[motor - MOTOR_FIRST_MOTOR].position);

			removeOffsets(&motor, sizeof(motor), &positionPtr);
			if (0 != (motorValues->speed & 0x0400)) /* this means a negative number */
			{
				g_motorPositions[motor - MOTOR_FIRST_MOTOR].speed = - (motorValues->speed & 0x03ff);
			} else {
				g_motorPositions[motor - MOTOR_FIRST_MOTOR].speed =   (motorValues->speed & 0x03ff);
			}

			if (0 != (motorValues->load & 0x0400)) /* this means a negative number */
			{
				g_motorPositions[motor - MOTOR_FIRST_MOTOR].load = - (motorValues->load & 0x03ff);
			} else {
				g_motorPositions[motor - MOTOR_FIRST_MOTOR].load =   (motorValues->load & 0x03ff);
			}
		}
	}
}

RPC_FUNCTION(getRawMotorPositions, "getRawMotorPositions", "retreive the motorvalues without offsets", uint8_t, motorPositionsArray_t, true, false)
	memcpy(o_param, g_motorPositionsNotAdjustedOffsets, sizeof(g_motorPositionsNotAdjustedOffsets));
}

static void onTriggerPushUp(void);
static void onTriggerPushUp(void)
{
	int *blubb = NULL;
	msgPump_getFreeBuffer(MSG_ID_TRIGGER_PUSH_MOTOR_VALUES, (void*)&blubb);
	if (NULL != blubb)
	{
		msgPump_postMessage(MSG_ID_TRIGGER_PUSH_MOTOR_VALUES, blubb);
	}
}

static void onPushDataToOdroid(const msgPump_MsgID_t msgID, const void *data)
{
	if ((MSG_ID_TRIGGER_PUSH_MOTOR_VALUES == msgID) &&
		(NULL != data))
	{
		static systemTime_t startTime = 0;
		if (0 == startTime)
		{
			startTime = getSystemTimeUS();
		}
		systemTime_t timeElapsed = getSystemTimeUS() - startTime;

		genericProtocol_sendMessage(1, 42, sizeof(g_motorPositions), &g_motorPositions);

		if (timeElapsed >= 2000000ULL)
		{
			uint8_t i = 0U;
			genericProtocol_sendMessage(1, 43, sizeof(g_motorInfos), &g_motorInfos);
			/* reset statistics */
			for (i = 0U; i <= MOTOR_LAST_MOTOR - MOTOR_FIRST_MOTOR; ++i)
			{
				g_motorInfos[i].missedReads     = 0U;
				g_motorInfos[i].successfullReads = 0U;
			}

			startTime = getSystemTimeUS();
			g_totalSuccessfullReads = 0U;
			g_totalUnsuccessfullReads = 0U;
		}
	}
}

static void onReadMotors(const msgPump_MsgID_t msgID, const void *data)
{
	if ((MSG_ID_TRIGGER_READ_MOTORS == msgID) &&
		(NULL != data))
	{

		bool readSuccess = true;


		for (uint8_t i = 0U; i < (sizeof(torsoMotors) / sizeof(torsoMotors[0])); ++i)
		{
			readSuccess &= dynamixel_read(torsoMotors[i], DYNAMIXEL_REGISTER_PRESENT_POSITION, 6, DYNAMIXEL_PORT_TORSO, &onMotorRead, DYNAMIXEL_TRANSACTION_PRIORITY_LOW);
		}

		for (uint8_t i = 0U; i < (sizeof(leftLegMotors) / sizeof(leftLegMotors[0])); ++i)
		{
			readSuccess &= dynamixel_read(leftLegMotors[i], DYNAMIXEL_REGISTER_PRESENT_POSITION, 6, DYNAMIXEL_PORT_LEFT_LEG, &onMotorRead, DYNAMIXEL_TRANSACTION_PRIORITY_LOW);
		}

		for (uint8_t i = 0U; i < (sizeof(rightLegMotors) / sizeof(rightLegMotors[0])); ++i)
		{
			readSuccess &= dynamixel_read(rightLegMotors[i], DYNAMIXEL_REGISTER_PRESENT_POSITION, 6, DYNAMIXEL_PORT_RIGHT_LEG, &onMotorRead, DYNAMIXEL_TRANSACTION_PRIORITY_LOW);
		}
	}
}

static void triggerReadMotors(void);
static void triggerReadMotors(void)
{
	int *blubb = NULL;
	msgPump_getFreeBuffer(MSG_ID_TRIGGER_READ_MOTORS, (void*)&blubb);
	if (NULL != blubb)
	{
		msgPump_postMessage(MSG_ID_TRIGGER_READ_MOTORS, blubb);
	}
}

#include "interfaces/systemTime.h"

#include <libopencm3/stm32/f4/dma.h>



static void motors_init(void);
MODULE_INIT_FUNCTION(motors, 9, motors_init)
static void motors_init(void)
{
	swTimer_registerOnTimer(&triggerReadMotors, MOTOR_READ_CYCLE_INTERVAL_MS, false);
	swTimer_registerOnTimer(&onTriggerPushUp, MOTOR_DISTRIBUTE_INTERVAL_MS, false);

	msgPump_registerOnMessage(MSG_ID_TRIGGER_PUSH_MOTOR_VALUES, &onPushDataToOdroid);
	msgPump_registerOnMessage(MSG_ID_TRIGGER_READ_MOTORS, &onReadMotors);

	for (uint8_t i = 0U; i < (sizeof(g_motorPositions) / sizeof(g_motorPositions[0])); ++i)
	{
		g_motorPositions[i].position = 0xffff;
	}
}

