/*
 * dynamixelForwarder.c
 *
 *  Created on: Mar 13, 2013
 *      Author: lutz
 */


#include <interfaces/dynamixelForwarder.h>
#include <interfaces/dynamixel.h>
#include <interfaces/motorIDs.h>
#include <interfaces/offsets.h>

#include <flawless/protocol/genericFlawLessProtocolApplication.h>
#include <flawless/protocol/genericFlawLessProtocol_Data.h>
#include <flawless/core/msg_msgPump.h>
#include <flawless/init/systemInitializer.h>
#include <flawless/config/msgIDs.h>

#define COM_2_MAIN_COMPUTER_LOGIC_INTERFACE_NR 1U

static dynamixelRegister_t g_currentReadRegister = 0;

typedef uint8_t dynamixelForwarderReturnPacket_t[DYNAMIXEL_MAX_RESPONSE_PACKE_LEN];

MSG_PUMP_DECLARE_MESSAGE_BUFFER_POOL(dynamixelforwarderISRSwapBuffer, dynamixelForwarderReturnPacket_t, 4, MSG_ID_DYNAMIXEL_FORWARDER_ISR_SWAP)

static void onMotorRead(bool success,
						const uint8_t *receivedData,
						uint8_t receivedDataCnt,
						uint8_t error,
						dynamixel_motorID_t motor,
						dynamixel_instruction_t instruction)
{
	UNUSED(error);
	UNUSED(motor);
	UNUSED(instruction);
	UNUSED(receivedDataCnt);
	if ((FALSE != success) && 0xff != g_currentReadRegister)
	{
		/* ugly ugly ugly application offsets */
		motorID_t destID    = receivedData[-3];
		uint8_t cnt      = receivedData[-2] - 2;
		uint8_t *payload = (uint8_t*) receivedData;
		int16_t applyOffsetAddrOffset = DYNAMIXEL_REGISTER_CW_ANGLE_LIMIT - g_currentReadRegister;
		if ((applyOffsetAddrOffset < cnt) && (applyOffsetAddrOffset >= 0))
		{
			uint16_t *registerData = (uint16_t*) (payload + applyOffsetAddrOffset);
			removeOffsets(&destID, sizeof(destID), &registerData);
		}
		applyOffsetAddrOffset = DYNAMIXEL_REGISTER_CCW_ANGLE_LIMIT - g_currentReadRegister;
		if ((applyOffsetAddrOffset < cnt) && (applyOffsetAddrOffset >= 0))
		{
			uint16_t *registerData = (uint16_t*) (payload + applyOffsetAddrOffset);
			removeOffsets(&destID, sizeof(destID), &registerData);
		}
		applyOffsetAddrOffset = DYNAMIXEL_REGISTER_PRESENT_POSITION - g_currentReadRegister;
		if ((applyOffsetAddrOffset < cnt) && (applyOffsetAddrOffset >= 0))
		{
			uint16_t *registerData = (uint16_t*) (payload + applyOffsetAddrOffset);
			removeOffsets(&destID, sizeof(destID), &registerData);
		}
		applyOffsetAddrOffset = DYNAMIXEL_REGISTER_GOAL_POSITION - g_currentReadRegister;
		if ((applyOffsetAddrOffset < cnt) && (applyOffsetAddrOffset >= 0))
		{
			uint16_t *registerData = (uint16_t*) (payload + applyOffsetAddrOffset);
			removeOffsets(&destID, sizeof(destID), &registerData);
		}

		/* recalculate the checksum */
		uint8_t checksum = 0;
		uint8_t i = 0;
		for (i = 0; i < cnt + 3; ++i)
		{
			checksum += receivedData[i - 3];
		}
		checksum = ~checksum;
		((uint8_t*)receivedData)[cnt] = checksum;

		msgPump_postMessage(MSG_ID_DYNAMIXEL_FORWARDER_ISR_SWAP, (void*)(receivedData - 5));
	}
	g_currentReadRegister = 0xff;
}

static void onForwardMotorReadMsg(msgPump_MsgID_t msgID, const void *data)
{
	if (MSG_ID_DYNAMIXEL_FORWARDER_ISR_SWAP == msgID && NULL != data)
	{
		const uint8_t *packet = (const uint8_t*) data;
		const uint8_t dataCnt = packet[3] + 4;
		genericProtocol_sendMessage(COM_2_MAIN_COMPUTER_LOGIC_INTERFACE_NR, DYNAMIXEL_FORWARD_PACKET_CHANNEL, dataCnt, data);
	}
}


static void dynamixelForwardingDataEP(const void *ipacket, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor);
GENERIC_PROTOCOL_ENDPOINT(dynamixelForwardingDataEP, DYNAMIXEL_FORWARD_PACKET_CHANNEL)
static void dynamixelForwardingDataEP(const void *ipacket, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor)
{
	UNUSED(i_packetLen);
	/* just listen from main computer */
	if (COM_2_MAIN_COMPUTER_LOGIC_INTERFACE_NR == i_interfaceDescriptor)
	{
		const uint8_t *packet = (const uint8_t*)ipacket;
		motorID_t destID    = packet[2];
		uint8_t length      = packet[3];
		dynamixelInstruction_t instruction = packet[4];

		const uint8_t *parameters = &(packet[5]);

		dynamixel_port_t port = DYNAMIXEL_PORT_ALL;
		if (GYRO == destID)
		{
			return;
		}
		if (MOTOR_ALL_MOTORS == destID)
		{
			port = DYNAMIXEL_PORT_ALL;
		} else if (destID <= DYNAMIXEL_LAST_TORSO_MOTOR)
		{
			port = DYNAMIXEL_PORT_TORSO;
		} else if (0 == (destID % 2) && (destID <= DYNAMIXEL_LAST_RIGHT_LEG_MOTOR))
		{
			port = DYNAMIXEL_PORT_RIGHT_LEG;
		} else if (destID <= DYNAMIXEL_LAST_LEFT_LEG_MOTOR)
		{
			port = DYNAMIXEL_PORT_LEFT_LEG;
		} else
		{
			port = DYNAMIXEL_PORT_LEFT_LEG;
		}

		switch (instruction)
		{
			case DYNAMIXEL_INSTRUCTION_ACTION:
				dynamixel_action(destID, port, DYNAMIXEL_TRANSACTION_PRIORITY_HIGH);
				break;
			case DYNAMIXEL_INSTRUCTION_PING:
				dynamixel_ping(destID, NULL, port, DYNAMIXEL_TRANSACTION_PRIORITY_HIGH);
				break;
			case DYNAMIXEL_INSTRUCTION_READ:
				{
					uint8_t reg = parameters[0];
					uint8_t cnt = parameters[1];
					g_currentReadRegister = reg;
					dynamixel_read(destID, reg, cnt, port, onMotorRead, DYNAMIXEL_TRANSACTION_PRIORITY_HIGH);
				}
				break;
			case DYNAMIXEL_INSTRUCTION_REG_WRITE:
				{
					uint8_t reg = parameters[0];
					const uint8_t *payload = &(parameters[1]);
					const uint8_t cnt = length - 3U;
					int16_t applyOffsetAddrOffset = DYNAMIXEL_REGISTER_CW_ANGLE_LIMIT - reg;
					if ((applyOffsetAddrOffset < cnt) && (applyOffsetAddrOffset >= 0))
					{
						uint16_t *registerData = (uint16_t*) (payload + applyOffsetAddrOffset);
						addOffsets(&destID, sizeof(destID), &registerData);
					}
					applyOffsetAddrOffset = DYNAMIXEL_REGISTER_CCW_ANGLE_LIMIT - reg;
					if ((applyOffsetAddrOffset < cnt) && (applyOffsetAddrOffset >= 0))
					{
						uint16_t *registerData = (uint16_t*) (payload + applyOffsetAddrOffset);
						addOffsets(&destID, sizeof(destID), &registerData);
					}
					applyOffsetAddrOffset = DYNAMIXEL_REGISTER_GOAL_POSITION - reg;
					if ((applyOffsetAddrOffset < cnt) && (applyOffsetAddrOffset >= 0))
					{
						uint16_t *registerData = (uint16_t*) (payload + applyOffsetAddrOffset);
						addOffsets(&destID, sizeof(destID), &registerData);
					}
					dynamixel_reg_write(destID, reg, payload, cnt, port, NULL, false, DYNAMIXEL_TRANSACTION_PRIORITY_HIGH);
				}
				break;
			case DYNAMIXEL_INSTRUCTION_RESET:
				dynamixel_reset(destID, port, DYNAMIXEL_TRANSACTION_PRIORITY_HIGH);
				break;
			case DYNAMIXEL_INSTRUCTION_SYNC_WRITE:
				{
					const uint8_t reg = parameters[0];
					const uint8_t L = parameters[1];
					const uint8_t N = (length - 4) / (L + 1);
					const uint8_t *payload = &(parameters[2]);
					static uint8_t motors[MOTOR_LAST_MOTOR] = {0};
					static const uint8_t *data[MOTOR_LAST_MOTOR] = {NULL};
					uint8_t i = 0U;
					for (i = 0U; i < N; ++i)
					{
						motors[i] = payload[i * (L + 1)];
						data[i] = &(payload[i * (L + 1) + 1]);
					}

					int16_t applyOffsetAddrOffset = DYNAMIXEL_REGISTER_CW_ANGLE_LIMIT - reg;
					if ((applyOffsetAddrOffset < L) && (applyOffsetAddrOffset >= 0))
					{
						for (i = 0U; i < N; ++i)
						{
							uint16_t *registerData = (uint16_t*) ((&data[i]) + applyOffsetAddrOffset);
							addOffsets(&(motors[i]), sizeof(motors[i]), &registerData);
						}
					}
					applyOffsetAddrOffset = DYNAMIXEL_REGISTER_CCW_ANGLE_LIMIT - reg;
					if ((applyOffsetAddrOffset < L) && (applyOffsetAddrOffset >= 0))
					{
						for (i = 0U; i < N; ++i)
						{
							uint16_t *registerData = (uint16_t*) ((&data[i]) + applyOffsetAddrOffset);
							addOffsets(&(motors[i]), sizeof(motors[i]), &registerData);
						}
					}
					applyOffsetAddrOffset = DYNAMIXEL_REGISTER_GOAL_POSITION - reg;
					if ((applyOffsetAddrOffset < L) && (applyOffsetAddrOffset >= 0))
					{
						for (i = 0U; i < N; ++i)
						{
							uint16_t *registerData = (uint16_t*) ((&data[i]) + applyOffsetAddrOffset);
							addOffsets(&(motors[i]), sizeof(motors[i]), &registerData);
						}
					}

					dynamixel_sync_write(motors, N, reg, data, L, port, DYNAMIXEL_TRANSACTION_PRIORITY_HIGH);
				}
				break;
			case DYNAMIXEL_INSTRUCTION_WRITE:
				{
					uint8_t reg = parameters[0];
					const uint8_t *payload = &(parameters[1]);
					uint8_t cnt = length - 3U;
					int16_t applyOffsetAddrOffset = DYNAMIXEL_REGISTER_CW_ANGLE_LIMIT - reg;
					if ((applyOffsetAddrOffset < cnt) && (applyOffsetAddrOffset >= 0))
					{
						uint16_t *registerData = (uint16_t*) (payload + applyOffsetAddrOffset);
						addOffsets(&destID, sizeof(destID), &registerData);
					}
					applyOffsetAddrOffset = DYNAMIXEL_REGISTER_CCW_ANGLE_LIMIT - reg;
					if ((applyOffsetAddrOffset < cnt) && (applyOffsetAddrOffset >= 0))
					{
						uint16_t *registerData = (uint16_t*) (payload + applyOffsetAddrOffset);
						addOffsets(&destID, sizeof(destID), &registerData);
					}
					applyOffsetAddrOffset = DYNAMIXEL_REGISTER_GOAL_POSITION - reg;
					if ((applyOffsetAddrOffset < cnt) && (applyOffsetAddrOffset >= 0))
					{
						uint16_t *registerData = (uint16_t*) (payload + applyOffsetAddrOffset);
						addOffsets(&destID, sizeof(destID), &registerData);
					}
					dynamixel_write(destID, reg, payload, cnt, port, NULL, false, DYNAMIXEL_TRANSACTION_PRIORITY_HIGH);
				}
				break;
			default:
				break;
		}
	}
}

static void dynamixelForwarderInit(void);
MODULE_INIT_FUNCTION(dynamixelForwarder, 7, dynamixelForwarderInit)
static void dynamixelForwarderInit(void)
{
	msgPump_registerOnMessage(MSG_ID_DYNAMIXEL_FORWARDER_ISR_SWAP, &onForwardMotorReadMsg);
}

