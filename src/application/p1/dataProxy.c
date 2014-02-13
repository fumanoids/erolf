/*
 * dataProxy.c
 *
 *  Created on: Feb 21, 2013
 *      Author: lutz
 */

#include <flawless/protocol/genericFlawLessProtocolApplication.h>
#include <flawless/protocol/genericFlawLessProtocol_Data.h>


/* used for IMU COMM */
static void proxy11Endpoint(const void *ipacket, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor);
GENERIC_PROTOCOL_ENDPOINT(proxy11Endpoint, 11)
static void proxy11Endpoint(const void *ipacket, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor)
{
	/* if we have received a packet from interface 0 we have to push it out on interface 1 and vice versa */
	if (0 == i_interfaceDescriptor)
	{
		genericProtocol_sendMessage(1, 11, i_packetLen, ipacket);
	} else if (1 == i_interfaceDescriptor)
	{
		genericProtocol_sendMessage(0, 11, i_packetLen, ipacket);
	}
}


#define POWER_BOARD_SWITCH_OFF_ENDPOINT 100
#define POWER_BOARD_GET_LIMITS_ENDPOINT 101
#define POWER_BOARD_SET_UPPER_VOLTAGE_ENDPOINT 102
#define POWER_BOARD_SET_LOWER_VOLTAGE_ENDPOINT 103
#define POWER_BOARD_MOTOR_SWITCH_OFF_ENDPOINT 105

/* used for power board configuration */
static void proxyPwrSwitchOffEndpoint(const void *ipacket, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor);
GENERIC_PROTOCOL_ENDPOINT(proxyPwrSwitchOffEndpoint, POWER_BOARD_SWITCH_OFF_ENDPOINT)
static void proxyPwrSwitchOffEndpoint(const void *ipacket, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor)
{
	/* if we have received a packet from interface 0 we have to push it out on interface 1 and vice versa */
	if (0 == i_interfaceDescriptor)
	{
		genericProtocol_sendMessage(1, POWER_BOARD_SWITCH_OFF_ENDPOINT, i_packetLen, ipacket);
	} else if (1 == i_interfaceDescriptor)
	{
		genericProtocol_sendMessage(0, POWER_BOARD_SWITCH_OFF_ENDPOINT, i_packetLen, ipacket);
	}
}

static void proxyPwrGetLimitsEndpoint(const void *ipacket, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor);
GENERIC_PROTOCOL_ENDPOINT(proxyPwrGetLimitsEndpoint, POWER_BOARD_GET_LIMITS_ENDPOINT)
static void proxyPwrGetLimitsEndpoint(const void *ipacket, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor)
{
	/* if we have received a packet from interface 0 we have to push it out on interface 1 and vice versa */
	if (0 == i_interfaceDescriptor)
	{
		genericProtocol_sendMessage(1, POWER_BOARD_GET_LIMITS_ENDPOINT, i_packetLen, ipacket);
	} else if (1 == i_interfaceDescriptor)
	{
		genericProtocol_sendMessage(0, POWER_BOARD_GET_LIMITS_ENDPOINT, i_packetLen, ipacket);
	}
}

static void proxyPwrSetUpperVEndpoint(const void *ipacket, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor);
GENERIC_PROTOCOL_ENDPOINT(proxyPwrSetUpperVEndpoint, POWER_BOARD_SET_UPPER_VOLTAGE_ENDPOINT)
static void proxyPwrSetUpperVEndpoint(const void *ipacket, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor)
{
	/* if we have received a packet from interface 0 we have to push it out on interface 1 and vice versa */
	if (0 == i_interfaceDescriptor)
	{
		genericProtocol_sendMessage(1, POWER_BOARD_SET_UPPER_VOLTAGE_ENDPOINT, i_packetLen, ipacket);
	} else if (1 == i_interfaceDescriptor)
	{
		genericProtocol_sendMessage(0, POWER_BOARD_SET_UPPER_VOLTAGE_ENDPOINT, i_packetLen, ipacket);
	}
}

static void proxyPwrSetLowerVEndpoint(const void *ipacket, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor);
GENERIC_PROTOCOL_ENDPOINT(proxyPwrSetLowerVEndpoint, POWER_BOARD_SET_LOWER_VOLTAGE_ENDPOINT)
static void proxyPwrSetLowerVEndpoint(const void *ipacket, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor)
{
	/* if we have received a packet from interface 0 we have to push it out on interface 1 and vice versa */
	if (0 == i_interfaceDescriptor)
	{
		genericProtocol_sendMessage(1, POWER_BOARD_SET_LOWER_VOLTAGE_ENDPOINT, i_packetLen, ipacket);
	} else if (1 == i_interfaceDescriptor)
	{
		genericProtocol_sendMessage(0, POWER_BOARD_SET_LOWER_VOLTAGE_ENDPOINT, i_packetLen, ipacket);
	}
}

static void proxyPwrMotorOffEndpoint(const void *ipacket, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor);
GENERIC_PROTOCOL_ENDPOINT(proxyPwrMotorOffEndpoint, POWER_BOARD_MOTOR_SWITCH_OFF_ENDPOINT)
static void proxyPwrMotorOffEndpoint(const void *ipacket, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor)
{
	/* if we have received a packet from interface 0 we have to push it out on interface 1 and vice versa */
	if (0 == i_interfaceDescriptor)
	{
		genericProtocol_sendMessage(1, POWER_BOARD_MOTOR_SWITCH_OFF_ENDPOINT, i_packetLen, ipacket);
	} else if (1 == i_interfaceDescriptor)
	{
		genericProtocol_sendMessage(0, POWER_BOARD_MOTOR_SWITCH_OFF_ENDPOINT, i_packetLen, ipacket);
	}
}


#define BEEPSOUND_ENDPOINT 31
static void proxyBeepSoundEndpoint(const void *ipacket, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor);
GENERIC_PROTOCOL_ENDPOINT(proxyBeepSoundEndpoint, BEEPSOUND_ENDPOINT)
static void proxyBeepSoundEndpoint(const void *ipacket, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor)
{
	/* if we have received a packet from interface 0 we have to push it out on interface 1 and vice versa */
	if (0 == i_interfaceDescriptor)
	{
		genericProtocol_sendMessage(1, BEEPSOUND_ENDPOINT, i_packetLen, ipacket);
	} else if (1 == i_interfaceDescriptor)
	{
		genericProtocol_sendMessage(0, BEEPSOUND_ENDPOINT, i_packetLen, ipacket);
	}
}
