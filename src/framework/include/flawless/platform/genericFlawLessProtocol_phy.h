/*
 * genericFlawlessProtocol_phy.h
 *
 *  Created on: Jun 10, 2012
 *      Author: lutz
 */

/*
 * this is the interface for the physiacal interface implementation of the generic flawless protocol.
 *
 */

#ifndef GENERICFLAWLESSPROTOCOL_PHY_H_
#define GENERICFLAWLESSPROTOCOL_PHY_H_

#include <flawless/stdtypes.h>
#include <flawless/config/flawLessProtocol_config.h>


/*
 * prototype for the raw send function
 *
 * @param data: the data to send.
 * @param packetLen: the amount of bytes to send
 */
typedef void (*phySendFunction_t)(const flawLessTransportSymbol_t *data, uint16_t packetLen);


/*
 * use functions like that to integrate your phyBusyFunction_t into the framework.
 * the number at the end indicates the index of the interface (must be 0 to FLAWLESS_PROTOCOL_PHY_INTERFACES_COUNT)
 *
 */
/*
 * void phySendFunction0(const flawLessTransportSymbol_t *data, uint16_t packetLen);
 */


#endif /* GENERICFLAWLESSPROTOCOL_PHY_H_ */
