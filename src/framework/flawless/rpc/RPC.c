/*
 * RPC.c
 *
 *  Created on: Apr 24, 2012
 *      Author: lutz
 */



#include <flawless/rpc/RPC.h>
#include <flawless/rpc/RPC_Interface.h>
#include <flawless/init/systemInitializer.h>
#include <flawless/stdtypes.h>

#include <flawless/core/msg_msgPump.h>
#include "../core/systemMsgIDs.h"

#include <flawless/misc/communication.h>
#include "../protocol/genericFlawLessProtocol.h"
#include <flawless/protocol/genericFlawLessProtocolApplication.h>
#include <flawless/protocol/genericFlawLessProtocol_Data.h>

extern const moduleRPCInfoHandle_t _moduleRPCHandlesBegin;
extern const moduleRPCInfoHandle_t _moduleRPCHandlesEnd;

MSG_PUMP_DECLARE_MESSAGE_BUFFER_POOL(rpcDispatcherBuffer, genericProtocoll_Packet_t, 4, MSG_ID_SYSTEM_RPC_DISPATCHER)

static int32_t g_rpcFunctionCount = 0U;

typedef struct tag_rpcResponseHeader
{
	RPC_ID_t rpcID;
}rpc_ResponseHeader_t;


static uint16_t strLen(const char *str);
static uint16_t strLen(const char *str)
{
	uint16_t c = 0;

	while('\0' != *(str + c))
	{
		c++;
	}

	return (c + 1); /* include trailing '\0' */
}

RPC_FUNCTION(rpcProbeSmallFunction, "RPCProbe", "A function which enumerates all implemented RPC functions on this device. \n Warning, this function does not return an uint8_t! And call it with the identifier -1", uint8_t, uint8_t, true, true)
	/* tells the calling application what is implemented on this device */
	RPC_ID_t i;
	rpc_ResponseHeader_t respHeader;
	const moduleRPCInfoHandle_t *rpcHandle = &_moduleRPCHandlesBegin;

	const RPC_ID_t rpcFuncCount = (RPC_ID_t)g_rpcFunctionCount;
	/* calculate total payloadlen */
	RPC_paramLength_t payloadLen = (uint16_t) (sizeof(respHeader) + sizeof(rpcFuncCount) +
											(g_rpcFunctionCount * (sizeof(RPC_ID_t) + sizeof(rpcHandle->i_parameterCanBeSmaller) + sizeof(rpcHandle->o_parameterCanBeSmaller) +
											sizeof(rpcHandle->i_parameterSize) + sizeof(rpcHandle->o_parameterSize))));

	for (i = 0U; i < rpcFuncCount; ++i)
	{
		payloadLen += strLen(rpcHandle->shortName);
		payloadLen += strLen(rpcHandle->in_parameterType);
		payloadLen += strLen(rpcHandle->o_parameterType);
		++rpcHandle;
	}

	genericProtocol_BeginTransmittingFrame(i_interface, payloadLen, RPC_SUB_PROTOCOL_IDENTIFIER);

	/*
	 * send rpc protocol header
	 */
	respHeader.rpcID = RPC_PROBE_SMALL_IDENTIFIER;
	genericProtocol_SendInsideFrame(i_interface, sizeof(respHeader), &respHeader);

	genericProtocol_SendInsideFrame(i_interface, sizeof(rpcFuncCount), &rpcFuncCount);
	/* for each rpc send the description */

	rpcHandle = &_moduleRPCHandlesBegin;
	for (i = 0U; i < rpcFuncCount; ++i)
	{
		uint16_t len = 0U;
		genericProtocol_SendInsideFrame(i_interface, sizeof(i), &i); /* send the rpc number */

		len = strLen(rpcHandle->shortName);
		genericProtocol_SendInsideFrame(i_interface, len, rpcHandle->shortName); /* send the rpc shortName */

		len = strLen(rpcHandle->in_parameterType);
		genericProtocol_SendInsideFrame(i_interface, len, rpcHandle->in_parameterType); /* send the rpc in_parameterName */

		len = strLen(rpcHandle->o_parameterType);
		genericProtocol_SendInsideFrame(i_interface, len, rpcHandle->o_parameterType); /* send the rpc o_parameterName */

		uint16_t networkShort = HTONS(rpcHandle->i_parameterSize);
		genericProtocol_SendInsideFrame(i_interface, sizeof(networkShort), &networkShort); /* send the size (in bytes) of the i_parameter */
		genericProtocol_SendInsideFrame(i_interface, sizeof(rpcHandle->i_parameterCanBeSmaller), &rpcHandle->i_parameterCanBeSmaller);

		networkShort = HTONS(rpcHandle->o_parameterSize);
		genericProtocol_SendInsideFrame(i_interface, sizeof(networkShort), &networkShort); /* send the size (in bytes) of the o_parameter */
		genericProtocol_SendInsideFrame(i_interface, sizeof(rpcHandle->o_parameterCanBeSmaller), &rpcHandle->o_parameterCanBeSmaller);
		++rpcHandle;
	}

	genericProtocol_EndTransmittingFrame(i_interface);
}

RPC_FUNCTION(rpcProbeDescriptiveFunction, "RPCProbeDescriptive", "A function which enumerates all implemented RPC functions on this device. The enumeration is more descriptive. \n WARNING, this function does not return an uint8_t! And call it with the identifier -2", uint8_t, uint8_t, true, true)

	/* tells the calling application what is implemented on this device */
	RPC_ID_t i;
	rpc_ResponseHeader_t respHeader;
	const moduleRPCInfoHandle_t *rpcHandle = &_moduleRPCHandlesBegin;

	const RPC_ID_t rpcFuncCount = (RPC_ID_t)g_rpcFunctionCount;
	/* calculate total payloadlen */
	RPC_paramLength_t payloadLen = (uint16_t) (sizeof(respHeader) + sizeof(rpcFuncCount) +
											(g_rpcFunctionCount * (sizeof(RPC_ID_t) + sizeof(rpcHandle->i_parameterCanBeSmaller) + sizeof(rpcHandle->o_parameterCanBeSmaller) +
											sizeof(rpcHandle->i_parameterSize) + sizeof(rpcHandle->o_parameterSize))));

	for (i = 0U; i < rpcFuncCount; ++i)
	{
		payloadLen += strLen(rpcHandle->shortName);
		payloadLen += strLen(rpcHandle->descriptiveName);
		payloadLen += strLen(rpcHandle->in_parameterType);
		payloadLen += strLen(rpcHandle->o_parameterType);
		++rpcHandle;
	}

	genericProtocol_BeginTransmittingFrame(i_interface, payloadLen, RPC_SUB_PROTOCOL_IDENTIFIER);
	/*
	 * send rpc protocol header
	 */
	respHeader.rpcID = RPC_PROBE_DESCRIPTIVE_IDENTIFIER;
	genericProtocol_SendInsideFrame(i_interface, sizeof(respHeader), &respHeader);

	genericProtocol_SendInsideFrame(i_interface, sizeof(rpcFuncCount), &rpcFuncCount);
	/* for each rpc send the description */

	rpcHandle = &_moduleRPCHandlesBegin;
	for (i = 0U; i < rpcFuncCount; ++i)
	{
		uint16_t len = 0U;
		genericProtocol_SendInsideFrame(i_interface, sizeof(i), &i); /* send the rpc number */

		len = strLen(rpcHandle->shortName);
		genericProtocol_SendInsideFrame(i_interface, len, rpcHandle->shortName); /* send the rpc shortName */

		len = strLen(rpcHandle->descriptiveName);
		genericProtocol_SendInsideFrame(i_interface, len, rpcHandle->descriptiveName); /* send the rpc descriptiveName */

		len = strLen(rpcHandle->in_parameterType);
		genericProtocol_SendInsideFrame(i_interface, len, rpcHandle->in_parameterType); /* send the rpc in_parameterName */

		len = strLen(rpcHandle->o_parameterType);
		genericProtocol_SendInsideFrame(i_interface, len, rpcHandle->o_parameterType); /* send the rpc o_parameterName */

		uint16_t networkShort = HTONS(rpcHandle->i_parameterSize);
		genericProtocol_SendInsideFrame(i_interface, sizeof(networkShort), &networkShort); /* send the size (in bytes) of the i_parameter */
		genericProtocol_SendInsideFrame(i_interface, sizeof(rpcHandle->i_parameterCanBeSmaller), &rpcHandle->i_parameterCanBeSmaller);

		networkShort = HTONS(rpcHandle->o_parameterSize);
		genericProtocol_SendInsideFrame(i_interface, sizeof(networkShort), &networkShort); /* send the size (in bytes) of the o_parameter */
		genericProtocol_SendInsideFrame(i_interface, sizeof(rpcHandle->o_parameterCanBeSmaller), &rpcHandle->o_parameterCanBeSmaller);
		++rpcHandle;
	}

	genericProtocol_EndTransmittingFrame(i_interface);
}



static void rpcEndPoint(const void *i_packet, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor);
GENERIC_PROTOCOL_ENDPOINT(rpcEndPoint, RPC_SUB_PROTOCOL_IDENTIFIER)
static void rpcEndPoint(const void *i_packet, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor)
{
	const uint8_t *packet = (const uint8_t*)i_packet;
	/* extract information */
	/* format: (int8_t)targetID, (char*)parameter */

	const RPC_ID_t *targetIDPtr = ((int8_t*)packet);
	const RPC_paramLength_t parameterLength = i_packetLen - (sizeof(RPC_ID_t) + sizeof(RPC_flags_t));
	const RPC_flags_t *rpcFlagsPtr = ((RPC_flags_t*)(packet + sizeof(RPC_ID_t)));
	const uint8_t *parameterPtr = (uint8_t*)(rpcFlagsPtr + sizeof(RPC_flags_t));

	const RPC_paramLength_t *parameterLengthPtr = &parameterLength;
	void *retValPtr = NULL;
	RPC_paramLength_t retValLen = 0U;
	if (g_rpcFunctionCount > *targetIDPtr)
	{
		if (*targetIDPtr < 0)
		{
			/* handle private (call to RPC manager) */
			switch(*targetIDPtr)
			{
				case RPC_PROBE_SMALL_IDENTIFIER:
					rpcProbeSmallFunction(parameterPtr, retValPtr, parameterLengthPtr, &retValLen, RPC_PROBE_SMALL_IDENTIFIER, i_interfaceDescriptor);
					break;
				case RPC_PROBE_DESCRIPTIVE_IDENTIFIER:
					rpcProbeDescriptiveFunction(parameterPtr, retValPtr, parameterLengthPtr, &retValLen, RPC_PROBE_DESCRIPTIVE_IDENTIFIER, i_interfaceDescriptor);
					break;
				default:
					/* discard (not implemented) */
					break;
			}
		} else
		{
			if (*targetIDPtr < g_rpcFunctionCount)
			{
				/* call to specific RPC function */
				const moduleRPCInfoHandle_t *handle = &(&_moduleRPCHandlesBegin)[*targetIDPtr];
				if ((parameterLength == handle->i_parameterSize) ||
						((false != handle->i_parameterCanBeSmaller) &&
						 (parameterLength <= handle->i_parameterSize)))
				{
					if (NULL != handle->rpcFunction)
					{
						void *retValPtr = handle->outParameterPtr;
						retValLen = handle->o_parameterSize;
						(void)(handle->rpcFunction)(parameterPtr, retValPtr, parameterLengthPtr, &retValLen, *targetIDPtr, i_interfaceDescriptor);

						if ((0 != ((*rpcFlagsPtr) & RPC_FLAGS_REQUIRES_RESPONSE)) &&
								(0 != retValLen) && /* dont send if there is nothing to tell*/
								(NULL != retValPtr))
						{
							rpc_ResponseHeader_t respHeader;
							const RPC_paramLength_t payLoadLen = retValLen + sizeof(rpc_ResponseHeader_t);
							respHeader.rpcID = *targetIDPtr;
							genericProtocol_BeginTransmittingFrame(i_interfaceDescriptor, payLoadLen, RPC_SUB_PROTOCOL_IDENTIFIER);
							genericProtocol_SendInsideFrame(i_interfaceDescriptor, sizeof(respHeader), &respHeader);
							genericProtocol_SendInsideFrame(i_interfaceDescriptor, retValLen, retValPtr);
							genericProtocol_EndTransmittingFrame(i_interfaceDescriptor);
						}
					}
				}
			}
		}
	}
}

void sendRPC_responseManually(RPC_ID_t senderID, RPC_paramLength_t paramLen, const void* param, const flawLessInterfaceDescriptor_t i_interfaceDescriptor)
{
	rpc_ResponseHeader_t respHeader;
	const RPC_paramLength_t payLoadLen = paramLen + sizeof(rpc_ResponseHeader_t);
	respHeader.rpcID = senderID;
	genericProtocol_BeginTransmittingFrame(i_interfaceDescriptor, payLoadLen, RPC_SUB_PROTOCOL_IDENTIFIER);
	genericProtocol_SendInsideFrame(i_interfaceDescriptor, sizeof(respHeader), &respHeader);
	genericProtocol_SendInsideFrame(i_interfaceDescriptor, paramLen, param);
	genericProtocol_EndTransmittingFrame(i_interfaceDescriptor);
}

static void rpcManager_init(void);
MODULE_INIT_FUNCTION(rpcManager, 5, rpcManager_init)
static void rpcManager_init(void)
{
	g_rpcFunctionCount = (int32_t)(&_moduleRPCHandlesEnd - &_moduleRPCHandlesBegin);
}

