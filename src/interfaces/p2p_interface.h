/*
 * p2p_interface.h
 *
 *  Created on: 15.01.2014
 *      Author: lutz
 */

#ifndef P2P_INTERFACE_H_
#define P2P_INTERFACE_H_


/**
 * baud
 */
#define P2P_USART_INTERFACE_SPEED 1000000ULL

/**
 * four byte times... that should be enough
 */
#define P2P_PACKET_PAUSE_INTERVAL_US (1000000 * 40 / P2P_USART_INTERFACE_SPEED)

#endif /* P2P_INTERFACE_H_ */
