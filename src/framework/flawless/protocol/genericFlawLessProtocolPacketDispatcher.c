/*
 * genericFlawLessProtocolPacketDispatcher.c
 *
 *  Created on: Jun 29, 2012
 *      Author: lutz
 */


#include <flawless/init/systemInitializer.h>
#include <flawless/core/msg_msgPump.h>

#include <flawless/protocol/genericFlawLessProtocolApplication.h>

#include "../core/systemMsgIDs.h"

#include "genericFlawLessProtocol.h"

extern const flawLess_genericProtocolHandle_t *_genericProtocolEndpointInternalHandlesBegin;
extern const flawLess_genericProtocolHandle_t *_genericProtocolEndpointInternalHandlesEnd;

static void onReceivedPacket_msgPumpCallback(msgPump_MsgID_t id, const void *buffer);
static void onReceivedPacket_msgPumpCallback(msgPump_MsgID_t id, const void *buffer)
{
	const genericProtocoll_Packet_t *packet = (const genericProtocoll_Packet_t*)buffer;
	const flawLess_genericProtocolHandle_t **iter  = &_genericProtocolEndpointInternalHandlesBegin;
//	const flawLess_genericProtocolHandle_t **iter2 = &_genericProtocolEndpointInternalHandlesBegin;
	const genericProtocol_subProtocolIdentifier_t targetID = packet->subProtocolID;
	if (MSG_ID_GENERIC_PROTOCOL_PACKET_READY == id)
	{
		while (iter < &_genericProtocolEndpointInternalHandlesEnd)
		{
			if (targetID == (*iter)->endpointIdentifier)
			{
				/* found the target endpoint */
				const uint8_t *packetForApplication = packet->packet;
				const uint16_t packetLen = packet->payloadLen;
				const flawLessInterfaceDescriptor_t interface = packet->interface;
				/* call the endpoint */
				((*iter)->callback)(packetForApplication, packetLen, interface);

				/* swap the handle pointer to increment the priority of this endpoint */
//				const flawLess_genericProtocolHandle_t *tmp = *iter;
//				*iter = *iter2;
//				*iter2 = tmp;
				break;
			}
//			iter2 = iter;
			iter++;
		}
	}
}

static void init(void);
MODULE_INIT_FUNCTION(genericFlawLessProtocolPacketDispatcher, 5, init)
static void init(void)
{
	const bool registerSuccess = msgPump_registerOnMessage(MSG_ID_GENERIC_PROTOCOL_PACKET_READY, &onReceivedPacket_msgPumpCallback);
	ASSERT(false != registerSuccess);
	const flawLess_genericProtocolHandle_t **iter = &_genericProtocolEndpointInternalHandlesBegin;
	while (iter < &_genericProtocolEndpointInternalHandlesEnd)
	{
		iter++;
	}
}

