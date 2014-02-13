/*
 * flawLessProtocol_config.h
 *
 *  Created on: Jun 10, 2012
 *      Author: lutz
 */

#ifndef FLAWLESSPROTOCOL_CONFIG_H_
#define FLAWLESSPROTOCOL_CONFIG_H_

#include <stdint.h>


/*
 * the maximum length of a single packet
 */
#define FLAWLESS_PROTOCOL_MAX_PACKET_LEN 128U

/*
 * those three chars are used for packet synchronization
 */

#define FLAWLESS_PROTOCOL_PACKET_CHAR_ESCAPE 0x3c
#define FLAWLESS_PROTOCOL_PACKET_CHAR_BEGINNING 0xf0
#define FLAWLESS_PROTOCOL_PACKET_CHAR_END 0x0f

/*
 * the amount of physical interfaces.
 * The generic protocol will handle each interface separately to gain maximum connectibility and portability
 * IMPORTANT: when changing this macro remember to change the appropriate section in the linker file
 */
#define FLAWLESS_PROTOCOL_PHY_INTERFACES_COUNT 10

typedef enum tag_flawLessProtocolStatus
{
	FLAWLESS_OK,
	FLAWLESS_ERROR,
	FLAWLESS_BUSY
} flawLessProtocolStatus_t;


/*
 * the interface descriptor for each phy interface.
 * Used by the higher layer to separate data sources
 */
typedef uint8_t flawLessInterfaceDescriptor_t;

typedef uint8_t genericProtocol_subProtocolIdentifier_t;


/*
 * the type of data to send
 */
typedef uint8_t flawLessTransportSymbol_t;

/*
 * include profiling of packet errors
 */
#define FLAWLESS_PROTOCOL_INTERFACE_STATISTICS 1
#define FLAWLESS_PROTOCOL_STATISTICS_INTERVAL_MS 5000

#endif /* FLAWLESSPROTOCOL_CONFIG_H_ */
