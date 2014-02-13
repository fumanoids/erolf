/*
 * logging.c
 *
 *  Created on: 11.01.2012
 *      Author: lutz
 */

#include <flawless/protocol/genericFlawLessProtocolApplication.h>
#include <flawless/protocol/genericFlawLessProtocol_Data.h>
#include <flawless/init/systemInitializer.h>

#include <flawless/misc/communication.h>

#include <flawless/platform/system.h>

#include <flawless/logging/logging.h>
#include <string.h>

/* don't compile that stuff in release mode */
#ifndef NO_LOG_OUTPUT


static bool g_logOutputInterfaces[FLAWLESS_PROTOCOL_PHY_INTERFACES_COUNT][LOG_LEVEL_CNT] = {{false}};

/* from linker magic */
extern const char _logStringsBegin;
extern const char _logStringsEnd;



void logFunction0(const logLevel_t level , const char * string)
{
	if((level < LOG_LEVEL_CNT) && (string >= &_logStringsBegin) && (string < &_logStringsEnd))
	{
		uint8_t i = 0U;
		for (i = 0U; i < FLAWLESS_PROTOCOL_PHY_INTERFACES_COUNT; ++i)
		{
			if (false != g_logOutputInterfaces[i][level])
			{
				/* send that logstring including the parameters */
				const logStringIndex_t index = HTONS((string - &_logStringsBegin));
				uint16_t payloadLen = sizeof(index) + sizeof(level);
				genericProtocol_BeginTransmittingFrame(i, payloadLen, LOGGING_OUT_SUB_PROTOCOL_ID);
				genericProtocol_SendInsideFrame(i, sizeof(level), &level);
				genericProtocol_SendInsideFrame(i, sizeof(index), &index);
				genericProtocol_EndTransmittingFrame(i);
			}
		}
	}
}

void logFunction1(const logLevel_t level , const char * string, int arg1)
{
	if((level < LOG_LEVEL_CNT) && (string >= &_logStringsBegin) && (string < &_logStringsEnd))
	{
		uint8_t interface = 0U;
		for (interface = 0U; interface < FLAWLESS_PROTOCOL_PHY_INTERFACES_COUNT; ++interface)
		{
			if (false != g_logOutputInterfaces[interface][level])
			{
				const int arg1Cnv = HTONL(arg1);
				/* send that logstring including the parameters */
				const logStringIndex_t index = HTONS((logStringIndex_t)(string - &_logStringsBegin));
				uint16_t payloadLen = sizeof(index) + sizeof(level) + sizeof(arg1);
				genericProtocol_BeginTransmittingFrame(interface, payloadLen, LOGGING_OUT_SUB_PROTOCOL_ID);
				genericProtocol_SendInsideFrame(interface, sizeof(level), &level);
				genericProtocol_SendInsideFrame(interface, sizeof(index), &index);
				genericProtocol_SendInsideFrame(interface, sizeof(arg1Cnv), &arg1Cnv);
				genericProtocol_EndTransmittingFrame(interface);
			}
		}
	}
}

void logFunction2(const logLevel_t level , const char * string, int arg1, int arg2)
{
	if((level < LOG_LEVEL_CNT) && (string >= &_logStringsBegin) && (string < &_logStringsEnd))
	{
		uint8_t i = 0U;
		for (i = 0U; i < FLAWLESS_PROTOCOL_PHY_INTERFACES_COUNT; ++i)
		{
			if (false != g_logOutputInterfaces[i][level])
			{
				const int arg1Cnv = HTONL(arg1);
				const int arg2Cnv = HTONL(arg2);
				/* send that logstring including the parameters */
				const logStringIndex_t index = HTONS((string - &_logStringsBegin));
				uint16_t payloadLen = sizeof(index) + sizeof(level) + sizeof(arg1) + sizeof(arg2);
				genericProtocol_BeginTransmittingFrame(i, payloadLen, LOGGING_OUT_SUB_PROTOCOL_ID);
				genericProtocol_SendInsideFrame(i, sizeof(level), &level);
				genericProtocol_SendInsideFrame(i, sizeof(index), &index);
				genericProtocol_SendInsideFrame(i, sizeof(arg1Cnv), &arg1Cnv);
				genericProtocol_SendInsideFrame(i, sizeof(arg2Cnv), &arg2Cnv);
				genericProtocol_EndTransmittingFrame(i);
			}
		}
	}
}

static const char g_describeArg[] = "describe:";
static const char g_enableArg[]   = "enable:";
static const char g_disableArg[]  = "disable:";

static bool sstrcmp(const char* i_str1, const char* i_str2)
{
	bool ret = false;
	while ((*i_str1 == *i_str2) &&
			('\0' != *i_str1))
	{
		++i_str1;
		++i_str2;
	}
	if (('\0' == *i_str1) &&
		('\0' == *i_str2))
	{
		ret = true;
	}
	return ret;
}

static void loggingEndpoint(const void *i_packet, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor);
GENERIC_PROTOCOL_ENDPOINT(loggingEndpoint, LOGGING_CTL_SUB_PROTOCOL_ID)
static void loggingEndpoint(const void *i_packet, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor)
{
	/* this endpoint understands some instructions */
	/* describe:
	 * 		send all logstrings to the requesting host (a log output will just send the index of the (format)string plus its argument(s))
	 * enable:
	 * 		enable a specific log-level (followed by a number between 0 and 5)
	 * disable:
	 * 		disable a specific log-level (followed by a number between 0 and 5)
	 */

	const char *data = (const char*) i_packet;
	const bool isDescribeInstr = sstrcmp(data, g_describeArg);
	if (false != isDescribeInstr)
	{
		/* send all the logstrings back to the host */
		const uint16_t payloadLen = (&_logStringsEnd - &_logStringsBegin);
		genericProtocol_sendMessage(i_interfaceDescriptor, LOGGING_CTL_SUB_PROTOCOL_ID, payloadLen, &_logStringsBegin);
	} else
	{
		const bool isEnableInstr = sstrcmp(data, g_enableArg);
		if (false != isEnableInstr)
		{
			/* set the loglevel as enabled */
			if (sizeof(g_enableArg) <= i_packetLen)
			{
				const uint8_t logLevel = data[sizeof(g_enableArg)];
				if (logLevel < LOG_LEVEL_CNT)
				{
					system_mutex_lock();
					g_logOutputInterfaces[i_interfaceDescriptor][logLevel] = true;
					system_mutex_unlock();
				}
			}
		} else
		{
			const bool isDisableInstr = sstrcmp(data, g_disableArg);
			if (false != isDisableInstr)
			{
				/* set the loglevel as disabled */
				if (sizeof(g_disableArg) <= i_packetLen)
				{
					const uint8_t logLevel = data[sizeof(g_disableArg)];
					if (logLevel < LOG_LEVEL_CNT)
					{
						system_mutex_lock();
						g_logOutputInterfaces[i_interfaceDescriptor][logLevel] = false;
						system_mutex_unlock();
					}
				}
			}
		}
	}
}

static void logging_init(void);
MODULE_INIT_FUNCTION(logging, 5, logging_init)
static void logging_init()
{
	uint8_t i = 0U, j = 0U;
	for (i = 0U; i < FLAWLESS_PROTOCOL_PHY_INTERFACES_COUNT; ++i)
	{
		for (j = 0U; i < LOG_LEVEL_CNT; ++i)
		{
			g_logOutputInterfaces[i][j] = false;
		}
	}
}

#endif
