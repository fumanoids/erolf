/*
 * genericFlawLessProtocol_Data.h
 *
 *  Created on: Jun 10, 2012
 *      Author: lutz
 */

#ifndef GENERICFLAWLESSPROTOCOL_DATA_H_
#define GENERICFLAWLESSPROTOCOL_DATA_H_

#include <flawless/config/flawLessProtocol_config.h>

/*
 * this is the abstract interface description for functions implemented inside the framework (data layer).
 *
 * Unless the communication subs are not linked into the target it is safe to call those functions:
 */


/*
 * A function implemented on the higher layer to indicate that the transmission of a packet was completed (with or without an error)
 *
 * @param interface: the interface where the transmission was completed
 * @param status: the status of the transmission (FLAWLESS_OK for successfull transmission)
 */
void flawLess_SentIndication(flawLessInterfaceDescriptor_t interface, flawLessProtocolStatus_t status);

/*
 * This function is already implemented in the framework and needs to be called for each received chunk (at least one byte) of data
 * The data gathered by the calling function can be discarded after calling this delegate function.
 * A copy of the relevant data will be made in the higher layer
 *
 * @param interface: the interface where data was received
 * @param data: the data received on that interface
 * @param packetLen: the amount of bytes received on this interface
 */
void flawLess_ReceiveIndication(flawLessInterfaceDescriptor_t interface, const flawLessTransportSymbol_t *data, uint16_t packetLen);



/*
 * used for asynchronous RPC calls for example
 */

/*
 * send an entire message in one chunk
 */
void genericProtocol_sendMessage(flawLessInterfaceDescriptor_t i_interface, genericProtocol_subProtocolIdentifier_t i_subProtocolID, uint16_t i_size, const void *i_buffer);

void genericProtocol_BeginTransmittingFrame(flawLessInterfaceDescriptor_t interface, uint16_t payloadLen, genericProtocol_subProtocolIdentifier_t subProtocolID);
void genericProtocol_SendInsideFrame(flawLessInterfaceDescriptor_t interface, uint16_t size, const void *buffer);
void genericProtocol_EndTransmittingFrame(flawLessInterfaceDescriptor_t interface);


#endif /* GENERICFLAWLESSPROTOCOL_DATA_H_ */
