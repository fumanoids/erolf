/*
 * msgProxy.c
 *
 *  Created on: 09.09.2012
 *      Author: lutz
 */

#include <flawless/protocol/msgProxy.h>

#include <flawless/stdtypes.h>
#include <flawless/init/systemInitializer.h>

#include <flawless/core/msg_msgPump.h>
#include <flawless/config/msgIDs.h>

#include <flawless/config/msgProxyConfig.h>

#include <flawless/protocol/genericFlawLessProtocolApplication.h>
#include <flawless/protocol/genericFlawLessProtocol_Data.h>

#include <flawless/config/flawLessProtocol_config.h>

extern  msgBufDescription_t _msgPumpBPHandlesBegin;
extern  msgBufDescription_t _msgPumpBPHandlesEnd;

static const extIDtoIntIDPair_t g_ext2intMap[] = MSG_PROXY_EXT_2_INT_MSG_ID_MAP;

static void msgCallback(msgPump_MsgID_t i_id, const void *i_data);
static void msgCallback(msgPump_MsgID_t i_id, const void *i_data)
{
	/* send that via generic protocol */
	const msgBufDescription_t *desc = &_msgPumpBPHandlesBegin;

	msgPump_MsgID_t externalID;

	uint8_t i;
	bool canSendMessage = false;

	for (i = 0U; i < (sizeof(g_ext2intMap) / sizeof(extIDtoIntIDPair_t)); ++i)
	{
		if (g_ext2intMap[i].internalID == i_id)
		{
			externalID = g_ext2intMap[i].externalID;
			canSendMessage = true;
			break;
		}
	}

	if (false != canSendMessage)
	{
		while (desc < &_msgPumpBPHandlesEnd)
		{
			if (i_id == desc->id)
			{
				/* get the length of that buffer element and send it */
				const msgBufMsgSize_t size = desc->msgSize;
				const uint16_t totalPackeLen = size + sizeof(msgID_t);
				uint8_t i = 0U;
				for (i = 0U; i < FLAWLESS_PROTOCOL_PHY_INTERFACES_COUNT; ++i)
				{
					genericProtocol_BeginTransmittingFrame(i, totalPackeLen, GENERIC_PROTOCOL_MSG_PROXY_ENDPOINT_ID);
					genericProtocol_SendInsideFrame(i, sizeof(externalID), &externalID);
					genericProtocol_SendInsideFrame(i, size, i_data);
					genericProtocol_EndTransmittingFrame(i);
				}
				break;
			}
			++desc;
		}
	}
}

static void incommingEndpoint(const void *ipacket, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor);
GENERIC_PROTOCOL_ENDPOINT(incommingEndpoint, GENERIC_PROTOCOL_MSG_PROXY_ENDPOINT_ID)
static void incommingEndpoint(const void *ipacket, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor)
{
	/* check if the incomming msgID and the size fits to any known local msg */
	const uint8_t *incommingPacket = (const uint8_t*) ipacket;
	const uint16_t *incommingIDp = ((const uint16_t*)incommingPacket);
	void *dataPtr = (void*)(incommingIDp + 1);
	const msgBufMsgSize_t incommingMessageSize = i_packetLen - sizeof(*incommingIDp);

	const msgBufDescription_t *desc = &_msgPumpBPHandlesBegin;

	msgPump_MsgID_t internalID;

	uint8_t i;
	bool canReceiveMessage = false;

	for (i = 0U; i < (sizeof(g_ext2intMap) / sizeof(extIDtoIntIDPair_t)); ++i)
	{
		if (g_ext2intMap[i].externalID == *incommingIDp)
		{
			internalID = g_ext2intMap[i].internalID;
			canReceiveMessage = true;
			break;
		}
	}

	if (false != canReceiveMessage)
	{
		while (desc < &_msgPumpBPHandlesEnd)
		{
			if (internalID == desc->id)
			{
				/* get the length of that buffer element and send it */
				const msgBufMsgSize_t size = desc->msgSize;
				if (size == incommingMessageSize)
				{
					/* yey we can accept that! */
					msgPump_postMessage(internalID, dataPtr);
				}
				break;
			}
			++desc;
		}
	}

	/* now we still need to send that message to the other devices connected to us */
	for (i = 0U; i < FLAWLESS_PROTOCOL_PHY_INTERFACES_COUNT; ++i)
	{
		/* dont send the message to the device which has just send it to us */
		if (i_interfaceDescriptor != i)
		{
			genericProtocol_sendMessage(i, GENERIC_PROTOCOL_MSG_PROXY_ENDPOINT_ID, i_packetLen, ipacket);
		}
	}
}


void msgProxy_addMsgForBroadcast(msgID_t i_msgID)
{
	msgPump_registerOnMessage(i_msgID, &msgCallback);
}

void msgProxy_rmMsgForBroadcast(msgID_t i_msgID)
{
	msgPump_unregisterFromMessage(i_msgID, &msgCallback);
}



