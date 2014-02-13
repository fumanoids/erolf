/*
 * genericFlawLessProtocolApplication.h
 *
 *  Created on: Jun 28, 2012
 *      Author: lutz
 */

#ifndef GENERICFLAWLESSPROTOCOLAPPLICATION_H_
#define GENERICFLAWLESSPROTOCOLAPPLICATION_H_


#include <flawless/config/flawLessProtocol_config.h>



typedef void (*flawLess_protocolApplicationCallback)(const void *ipacket, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor);

typedef struct tag_flawLess_genericProtocolEndpontHandle
{
	/* the endpoint identifier. Works similar to a port in IP */
	const genericProtocol_subProtocolIdentifier_t endpointIdentifier;
	const uint8_t padding;
	const uint16_t padding2;
	const uint32_t padding3;
	/* the application callback to this endpoint */
	const flawLess_protocolApplicationCallback callback;
} flawLess_genericProtocolHandle_t;

/*
 * with this macro an application can register a function as an application callback to packets
 * received to a specific subprotocol ID (a number between 0 and 127).
 * Only IDs from 10 should be used for application software.
 * The others are reserved for the framework.
 */
#define GENERIC_PROTOCOL_ENDPOINT(function, subProtocolID)\
	__attribute__ ((unused)) \
	__attribute__ ((section(".genericProtocolEndpointHandles"))) \
	flawLess_genericProtocolHandle_t function ##_handle = {subProtocolID, 0U, 0U, 0U, function}; \
	__attribute__ ((unused)) \
	__attribute__ ((section(".genericProtocolEndpointInternalHandles"))) \
	flawLess_genericProtocolHandle_t *function ##_internalHandle = &(function ##_handle);



#endif /* GENERICFLAWLESSPROTOCOLAPPLICATION_H_ */
