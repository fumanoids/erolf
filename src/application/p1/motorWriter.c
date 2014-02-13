/*
 * motorWriter.c
 *
 *  Created on: Apr 10, 2013
 *      Author: lutz
 */


#include <flawless/init/systemInitializer.h>

#include <flawless/protocol/genericFlawLessProtocolApplication.h>
#include <flawless/timer/swTimer.h>

#include <flawless/core/msg_msgPump.h>
#include <flawless/config/msgIDs.h>

#include <flawless/rpc/RPC.h>

#include <interfaces/motorIDs.h>
#include <interfaces/offsets.h>
#include <interfaces/motorAssociations.h>

#include <interfaces/dynamixel_config.h>
#include <interfaces/dynamixel.h>

#include <interfaces/configuration.h>

#include <string.h>


MSG_PUMP_DECLARE_MESSAGE_BUFFER_POOL(motorButtonSwitchBP, uint8_t, 3, MSG_ID_USER_MOTOR_SWITCH)

#define SEND_COMPLIANCE_MARGIN_DELAY_MS 250U

#define COM_2_MAIN_COMPUTER_LOGIC_INTERFACE_NR 1U

/***********************************************************************************/
/***************************** motor initializazion values *************************/
/***********************************************************************************/

#define COMPLIANCE_MARGIN 1U
#define COMPLIANCE_SLOPE  16U
#define ALARM_SHUTDOWN 36U
#define TEMPERATURE_LIMIT 70U

#define DYNAMIXEL_RANGE_MAX 1023U

static const motorID_t allMotors[] = MOTORS_ALL;
static const motorID_t torsoMotors[] = MOTORS_TORSO;
static const motorID_t leftLegMotors[] = MOTORS_LEFT_LEG;
static const motorID_t rightLegMotors[] = MOTORS_RIGHT_LEG;

typedef struct tag_motorSetupValues
{
	uint8_t complianceMarginCW, complianceMarginCCW;
	uint8_t complianceSlopeCW, complianceSlopeCCW;
} motorSetupValue_t;

typedef motorSetupValue_t motorSetupValues_t[MOTOR_LAST_MOTOR];

static const motorSetupValues_t motorSetupDefaultValues =
{
		{0, 0, 3, 3}, /* head */
		{0, 0, 3, 3}, /* head */

		{2, 2, 16, 16}, /* arm */
		{2, 2, 16, 16}, /* arm */
		{2, 2, 16, 16}, /* arm */
		{2, 2, 16, 16}, /* arm */
		{2, 2, 16, 16}, /* arm */
		{2, 2, 16, 16}, /* arm */

		{1, 1, 16, 16}, /* leg */
		{1, 1, 16, 16}, /* leg */
		{1, 1, 16, 16}, /* leg */
		{1, 1, 16, 16}, /* leg */
		{1, 1, 16, 16}, /* leg */
		{1, 1, 16, 16}, /* leg */
		{1, 1, 16, 16}, /* leg */
		{1, 1, 16, 16}, /* leg */
		{1, 1, 16, 16}, /* leg */
		{1, 1, 16, 16}, /* leg */
		{1, 1, 16, 16}, /* leg */
		{1, 1, 16, 16}  /* leg */
};

CONFIG_VARIABLE(motorSetupValues_t, g_motorSetupValues, &motorSetupDefaultValues);


static void initializeMotors(void);


RPC_FUNCTION(setMotorSetupValues, "setMotorSetupValues", "setup the compliance margins and slopes for all motors", motorSetupValues_t, uint8_t, false, true)
	memcpy(g_motorSetupValues, i_param, sizeof(g_motorSetupValues));
	config_updateToFlash();
	initializeMotors();
	*((uint8_t*)o_param) = 1U;
}

typedef struct tag_motorPositionAnsSpeed_t {
	uint16_t position, speed;
} motorPositionAnsSpeed_t;

static void writeMotorValues(const void *i_packet, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor);
GENERIC_PROTOCOL_ENDPOINT(writeMotorValues, 42)
static void writeMotorValues(const void *i_packet, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor)
{
	if ((COM_2_MAIN_COMPUTER_LOGIC_INTERFACE_NR == i_interfaceDescriptor) &&
		(NULL != i_packet) &&
		(80 == i_packetLen))
	{
		const motorPositionAnsSpeed_t *values = (const motorPositionAnsSpeed_t*) i_packet;

		/* Write all torso motors */
		{
			uint8_t *dataPtrs[sizeof(torsoMotors)];
			motorID_t motorIDs[sizeof(torsoMotors)];
			uint8_t i;
			uint8_t motorCnt = 0;
			for (i = 0; i < sizeof(torsoMotors); ++i)
			{
				if ((values[torsoMotors[i] - 1U].position <= DYNAMIXEL_RANGE_MAX) && values[torsoMotors[i] - 1U].speed <= DYNAMIXEL_RANGE_MAX)
				{
					dataPtrs[motorCnt] = (uint8_t*)&(values[torsoMotors[i] - 1U]);
					motorIDs[motorCnt] = torsoMotors[i];
					++motorCnt;
				}
			}
			addOffsets(motorIDs, motorCnt, (uint16_t**)dataPtrs);
			dynamixel_sync_write(motorIDs, motorCnt, DYNAMIXEL_REGISTER_GOAL_POSITION, (const uint8_t**)dataPtrs, 4, DYNAMIXEL_PORT_TORSO, DYNAMIXEL_TRANSACTION_PRIORITY_HIGH);
		}

		/* Write all left leg motors */
		{
			uint8_t *dataPtrs[sizeof(leftLegMotors)];
			motorID_t motorIDs[sizeof(leftLegMotors)];
			uint8_t i;
			uint8_t motorCnt = 0;
			for (i = 0; i < sizeof(leftLegMotors); ++i)
			{
				if ((values[leftLegMotors[i] - 1U].position <= DYNAMIXEL_RANGE_MAX) && values[leftLegMotors[i] - 1U].speed <= DYNAMIXEL_RANGE_MAX)
				{
					dataPtrs[motorCnt] = (uint8_t*)&(values[leftLegMotors[i] - 1U]);
					motorIDs[motorCnt] = leftLegMotors[i];
					++motorCnt;
				}
			}
			addOffsets(motorIDs, motorCnt, (uint16_t**)dataPtrs);
			dynamixel_sync_write(motorIDs, motorCnt, DYNAMIXEL_REGISTER_GOAL_POSITION, (const uint8_t**)dataPtrs, 4, DYNAMIXEL_PORT_LEFT_LEG, DYNAMIXEL_TRANSACTION_PRIORITY_HIGH);
		}

		/* Write all right leg motors */
		{
			uint8_t *dataPtrs[sizeof(rightLegMotors)];
			motorID_t motorIDs[sizeof(rightLegMotors)];
			uint8_t i;
			uint8_t motorCnt = 0;
			for (i = 0; i < sizeof(rightLegMotors); ++i)
			{
				if ((values[rightLegMotors[i] - 1U].position <= DYNAMIXEL_RANGE_MAX) && values[rightLegMotors[i] - 1U].speed <= DYNAMIXEL_RANGE_MAX)
				{
					dataPtrs[motorCnt] = (uint8_t*)&(values[rightLegMotors[i] - 1U]);
					motorIDs[motorCnt] = rightLegMotors[i];
					++motorCnt;
				}
			}
			addOffsets(motorIDs, motorCnt, (uint16_t**)dataPtrs);
			dynamixel_sync_write(motorIDs, motorCnt, DYNAMIXEL_REGISTER_GOAL_POSITION, (const uint8_t**)dataPtrs, 4, DYNAMIXEL_PORT_RIGHT_LEG, DYNAMIXEL_TRANSACTION_PRIORITY_HIGH);
		}
	}
}


static void setupMotors(void);
static void setupMotors(void)
{
	/* send a "shut up" message to every motor */
	uint8_t returnDelayTime = 40U;
	uint8_t statusReturnLevel = 1U;
	uint16_t movingSpeed = 0U;

	for (dynamixel_port_t port = DYNAMIXEL_PORT_1; port < DYNAMIXEL_PORT_CNT; ++port)
	{
		dynamixel_write(DYNAMIXEL_BROADCAST_ID, DYNAMIXEL_REGISTER_STATUS_RETURN_LEVEL, (uint8_t*)&statusReturnLevel, sizeof(statusReturnLevel), port, NULL, false, DYNAMIXEL_TRANSACTION_PRIORITY_HIGH);
		dynamixel_write(DYNAMIXEL_BROADCAST_ID, DYNAMIXEL_REGISTER_RETURN_DELAY_TIME, (uint8_t*)&returnDelayTime, sizeof(returnDelayTime), port, NULL, false, DYNAMIXEL_TRANSACTION_PRIORITY_HIGH);
		dynamixel_write(DYNAMIXEL_BROADCAST_ID, DYNAMIXEL_REGISTER_MOVING_SPEED, (uint8_t*)&movingSpeed, sizeof(movingSpeed), port, NULL, false, DYNAMIXEL_TRANSACTION_PRIORITY_HIGH);
	}
}

static void initializeMotors(void)
{
	const uint8_t *dataPtrs[MOTOR_LAST_MOTOR];
	uint8_t i;

	setupMotors();

	/* torso motors */
	for (i = 0U; i < sizeof(torsoMotors); ++i)
	{
		dataPtrs[i] = (const uint8_t*)&(g_motorSetupValues[i + MOTORS_FIRST_MOTOR_TORSO - MOTOR_FIRST_MOTOR]);
	}
	dynamixel_sync_write(torsoMotors, sizeof(torsoMotors), DYNAMIXEL_REGISTER_CW_COMPLIANCE_MARGIN, dataPtrs, sizeof(motorSetupValue_t), DYNAMIXEL_PORT_TORSO, DYNAMIXEL_TRANSACTION_PRIORITY_HIGH);

	/* left leg */
	for (i = 0U; i < sizeof(leftLegMotors); ++i)
	{
		dataPtrs[i] = (const uint8_t*)&(g_motorSetupValues[i + MOTORS_FIRST_MOTOR_LEFT_LEG - MOTOR_FIRST_MOTOR]);
	}
	dynamixel_sync_write(leftLegMotors, sizeof(leftLegMotors), DYNAMIXEL_REGISTER_CW_COMPLIANCE_MARGIN, dataPtrs, sizeof(motorSetupValue_t), DYNAMIXEL_PORT_LEFT_LEG, DYNAMIXEL_TRANSACTION_PRIORITY_HIGH);

	/* right leg */
	for (i = 0U; i < sizeof(rightLegMotors); ++i)
	{
		dataPtrs[i] = (const uint8_t*)&(g_motorSetupValues[i + MOTORS_FIRST_MOTOR_RIGHT_LEG - MOTOR_FIRST_MOTOR]);
	}
	dynamixel_sync_write(rightLegMotors, sizeof(rightLegMotors), DYNAMIXEL_REGISTER_CW_COMPLIANCE_MARGIN, dataPtrs, sizeof(motorSetupValue_t), DYNAMIXEL_PORT_RIGHT_LEG, DYNAMIXEL_TRANSACTION_PRIORITY_HIGH);
}

static void onUserButtonSwitchChanged(msgPump_MsgID_t msgID, const void* buf);
static void onUserButtonSwitchChanged(msgPump_MsgID_t msgID, const void* buf)
{
	UNUSED(buf);
	if (MSG_ID_USER_MOTOR_SWITCH == msgID)
	{
		/* soon send the compliance margin */
		swTimer_registerOnTimer(&initializeMotors, SEND_COMPLIANCE_MARGIN_DELAY_MS, true);
	}
}

static void motor_WriterInitFunction(void);
MODULE_INIT_FUNCTION(motorWriter, 9, motor_WriterInitFunction)
static void motor_WriterInitFunction(void)
{
	msgPump_registerOnMessage(MSG_ID_USER_MOTOR_SWITCH, &onUserButtonSwitchChanged);
	swTimer_registerOnTimer(&initializeMotors, SEND_COMPLIANCE_MARGIN_DELAY_MS, true);
}


