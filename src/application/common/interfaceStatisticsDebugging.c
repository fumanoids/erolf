/*
 * interfaceStatisticsDebugging.c
 *
 *  Created on: 18.01.2014
 *      Author: lutz
 */

#include <flawless/core/msg_msgPump.h>
#include <flawless/init/systemInitializer.h>
#include <flawless/platform/system.h>
#include <flawless/logging/logging.h>
#include <flawless/protocol/genericFlawLessProtocol_Data.h>
#include <flawless/config/flawLessProtocol_config.h>

#include <framework/flawless/core/systemMsgIDs.h>


#ifdef FLAWLESS_PROTOCOL_INTERFACE_STATISTICS

typedef struct tag_genericProtocolInterfaceStatistics
{
	uint16_t totalPackets;
	uint16_t checkSumErrors;
	uint16_t syncErrors;
	uint16_t sizeErrors;
} genericProtocolInterfaceStatistics_t;

typedef genericProtocolInterfaceStatistics_t genericProtocolInterfaceStatisticsArray_t[3];

static void onInterfaceStatistics(msgPump_MsgID_t msgID, const void *data)
{
	UNUSED(msgID);
	UNUSED(data);
	const genericProtocolInterfaceStatistics_t *statistics = (const genericProtocolInterfaceStatistics_t*)data;
	for (uint8_t i = 0; i < 3; ++i)
	{
		const genericProtocolInterfaceStatistics_t *interfaceStats = &(statistics[i]);
		if ((interfaceStats->checkSumErrors != 0) ||
			(interfaceStats->sizeErrors != 0) ||
			(interfaceStats->syncErrors != 0))
		{
			LOG_WARNING_2("Interface Stats bad %d, %d, %d", interfaceStats->checkSumErrors, interfaceStats->sizeErrors);
		}
	}
}

static void interfaceStatisticsDebugging_init(void);
MODULE_INIT_FUNCTION(interfaceStatistics, 9, interfaceStatisticsDebugging_init);
static void interfaceStatisticsDebugging_init(void)
{
	msgPump_registerOnMessage(MSG_ID_GENERIC_PROTOCOL_PACKET_STATISTICS, &onInterfaceStatistics);
}

#endif
