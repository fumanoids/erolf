/*
 * dynamixel_config.h
 *
 *  Created on: Dec 1, 2012
 *      Author: lutz
 */

#ifndef DYNAMIXEL_CONFIG_H_
#define DYNAMIXEL_CONFIG_H_


typedef enum tag_instruction
{
	DYNAMIXEL_INSTRUCTION_PING        = 0x1,
	DYNAMIXEL_INSTRUCTION_READ        = 0x2,
	DYNAMIXEL_INSTRUCTION_WRITE       = 0x3,
	DYNAMIXEL_INSTRUCTION_REG_WRITE   = 0x4,
	DYNAMIXEL_INSTRUCTION_ACTION      = 0x5,
	DYNAMIXEL_INSTRUCTION_RESET       = 0x6,
	DYNAMIXEL_INSTRUCTION_SYNC_WRITE  = 0x83
} dynamixelInstruction_t;


enum tag_dynamixel_register
{
	DYNAMIXEL_REGISTER_ID = 0x3,
	DYNAMIXEL_REGISTER_BAUD_RATE = 0x4,
	DYNAMIXEL_REGISTER_RETURN_DELAY_TIME = 0x5,
	DYNAMIXEL_REGISTER_CW_ANGLE_LIMIT = 0x6,
	DYNAMIXEL_REGISTER_CCW_ANGLE_LIMIT = 0x8,
	DYNAMIXEL_REGISTER_TEMP_H_LIMIT = 0xb,
	DYNAMIXEL_REGISTER_VOLT_L_LIMIT = 0xc,
	DYNAMIXEL_REGISTER_VOLT_H_LIMIT = 0xd,
	DYNAMIXEL_REGISTER_MAX_TORQUE = 0xe,
	DYNAMIXEL_REGISTER_STATUS_RETURN_LEVEL = 0x10,
	DYNAMIXEL_REGISTER_ALARM_LED = 0x11,
	DYNAMIXEL_REGISTER_ALARM_SHUTDOWN = 0x12,
	DYNAMIXEL_REGISTER_TORQUE_ENABLED = 0x18,
	DYNAMIXEL_REGISTER_LED = 0x19,
	DYNAMIXEL_REGISTER_CW_COMPLIANCE_MARGIN = 0x1a,
	DYNAMIXEL_REGISTER_CCW_COMPLIANCE_MARGIN = 0x1b,
	DYNAMIXEL_REGISTER_CW_COMPLIANCE_SLOPE = 0x1c,
	DYNAMIXEL_REGISTER_CCW_COMPLIANCE_SLOPE = 0x1d,
	DYNAMIXEL_REGISTER_GOAL_POSITION = 0x1e,
	DYNAMIXEL_REGISTER_MOVING_SPEED = 0x20,
	DYNAMIXEL_REGISTER_TORQUE_LIMIT = 0x22,
	DYNAMIXEL_REGISTER_PRESENT_POSITION = 0x24,
	DYNAMIXEL_REGISTER_PRESENT_SPEED    = 0x26,
	DYNAMIXEL_REGISTER_PRESENT_LOAD = 0x28,
	DYNAMIXEL_REGISTER_RESENT_VOLTAGE = 0x2a,
	DYNAMIXEL_REGISTER_PRESENT_TEMPERATURE = 0x2b,
	DYNAMIXEL_REGISTER_REGISTERED_INSTRUCTION = 0x2c,
	DYNAMIXEL_REGISTER_LOCK = 0x2f,
	DYNAMIXEL_REGISTER_PUNCH = 0x30,
};

typedef uint8_t dynamixelRegister_t;

typedef enum tag_dynamixel_port
{
	DYNAMIXEL_PORT_1 = 0, /* torso*/
	DYNAMIXEL_PORT_2 = 1, /* left leg */
	DYNAMIXEL_PORT_3 = 2,  /* right leg */
	DYNAMIXEL_PORT_CNT,

	DYNAMIXEL_PORT_ALL = 0xff
} dynamixel_port_t;

#define DYNAMIXEL_PORT_TORSO DYNAMIXEL_PORT_1
#define DYNAMIXEL_PORT_LEFT_LEG DYNAMIXEL_PORT_2
#define DYNAMIXEL_PORT_RIGHT_LEG DYNAMIXEL_PORT_3

#define DYNAMIXEL_MAX_RESPONSE_DELAY_MS 1U

#define DYNAMIXEL_BROADCAST_ID (0xfe)

typedef uint8_t dynamixel_motorID_t;

typedef uint8_t dynamixel_instruction_t;

typedef uint8_t dynamixel_register_t;

#define DYNAMIXEL_MAX_RESPONSE_PACKE_LEN 20


#endif /* DYNAMIXEL_CONFIG_H_ */
