/*
 * transport_config.h
 *
 *  Created on: Jun 9, 2012
 *      Author: lutz
 */

#ifndef TRANSPORT_CONFIG_H_
#define TRANSPORT_CONFIG_H_

#include <libopencm3/stm32/usart.h>

typedef enum {
	/* insert phyiscal transport identifiers here */
	TRANSPORT_DEVICE_UNUSED= USART1,
	TRANSPORT_DEVICE_DEBUG = USART3,
	TRANSPORT_DEVICE_MAINBOARD = USART2
} transport_device_values_t;

#define LOGIC_INTERFACE_MAIN_BOARD 0
#define LOGIC_INTERFACE_DEBUG 1

#define TRANSPORT_DEVICE_MAINBOARD_SPEED 115200LLU
#define TRANSPORT_DEVICE_DEBUG_SPEED 115200LLU

#endif /* TRANSPORT_CONFIG_H_ */
