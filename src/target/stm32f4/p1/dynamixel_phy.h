/*
 * dynamixel_phy.h
 *
 *  Created on: Dec 10, 2012
 *      Author: lutz
 */

#ifndef DYNAMIXEL_PHY_H_
#define DYNAMIXEL_PHY_H_

#include <flawless/stdtypes.h>
#include <interfaces/dynamixel_config.h>


/*
 * logic interface port (wraps to physical RS485 interface)
 */

typedef enum tag_dynamixelInterfaceMode
{
	DYNAMIXEL_INTERFACE_MODE_RX = 0U,
	DYNAMIXEL_INTERFACE_MODE_TX = 1U
} dynamixel_interfaceMode_t;

typedef void (*dynamixel_receive_callback_t)(dynamixel_port_t port, uint8_t *receivedData, uint8_t len);
typedef void (*dynamixel_txComplete_callback_t)(dynamixel_port_t port);

void dynamixel_phy_setRxCompleteCallback(dynamixel_receive_callback_t i_callback);

void dynamixel_phy_setTXCCallback(dynamixel_txComplete_callback_t i_callback);

void dynamixel_phy_resetInterface(dynamixel_port_t port);
bool dynamixel_phy_send(dynamixel_port_t port, uint8_t *i_data, uint8_t len);
bool dynamixel_phy_receive(dynamixel_port_t port, uint8_t *i_data, uint8_t len);


#endif /* DYNAMIXEL_PHY_H_ */
