#ifndef GENERIC_FLAW_LESS_PROTOCOL_H
#define GENERIC_FLAW_LESS_PROTOCOL_H

#include <flawless/stdtypes.h>
#include <flawless/config/flawLessProtocol_config.h>

/*
 * this protocol handles communication to the framework
 * this is the base stub for event based message handling.
 *
 * Just pass each Byte received by an interface to this protocol
 */

typedef uint16_t genericProtocol_payloadLen_t;
typedef uint8_t genericProtocol_Checksum_t;

#define GENERIC_PROTOCOL_PAYLOAD_LEN_SIZE       sizeof(genericProtocol_payloadLen_t)
#define GENERIC_PROTOCOL_MAX_TOTAL_SIZE         (GENERIC_PROTOCOL_MAX_PAYLOAD_LEN + GENERIC_PROTOCOL_HEADER_SIZE)


/* buffer for one packet */
typedef struct tag_genericProtocol_Packet
{
	genericProtocol_payloadLen_t payloadLen;
	flawLessInterfaceDescriptor_t interface;
	genericProtocol_subProtocolIdentifier_t subProtocolID;
	flawLessTransportSymbol_t packet[FLAWLESS_PROTOCOL_MAX_PACKET_LEN];
}genericProtocoll_Packet_t;


#endif /* GENERIC_FLAW_LESS_PROTOCOL_H */
