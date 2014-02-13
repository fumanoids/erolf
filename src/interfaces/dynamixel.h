/*
 * dynamixel.h
 *
 *  Created on: Dec 1, 2012
 *      Author: lutz
 */

#ifndef DYNAMIXEL_H_
#define DYNAMIXEL_H_

#include <flawless/stdtypes.h>


#include <interfaces/dynamixel_config.h>
#include <interfaces/motorIDs.h>


typedef struct tag_dynamixelStatusPacket
{
	uint8_t payload[DYNAMIXEL_MAX_RESPONSE_PACKE_LEN];
	motorID_t motorID;
	uint8_t error;
	uint8_t payloadLen;
} dynamixelStatusPacket_t;

typedef enum tag_transactionPriority
{
	DYNAMIXEL_TRANSACTION_PRIORITY_LOW,
	DYNAMIXEL_TRANSACTION_PRIORITY_HIGH,
	DYNAMIXEL_TRANSACTION_PRIORITY_CNT
} transactionPriority_t;


typedef void (*dynamixel_Callback_t)(bool success,
								const uint8_t *receivedData,
								uint8_t receivedDataCnt,
								uint8_t motorError,
								dynamixel_motorID_t device,
								dynamixel_instruction_t instruction);


bool dynamixel_ping(dynamixel_motorID_t addr,
					dynamixel_Callback_t callback,
					dynamixel_port_t port,
					transactionPriority_t priority);

bool dynamixel_read(dynamixel_motorID_t addr,
					dynamixel_register_t reg,
					uint8_t cnt,
					dynamixel_port_t port,
					dynamixel_Callback_t callback,
					transactionPriority_t priority);

bool dynamixel_write(dynamixel_motorID_t addr,
					dynamixel_register_t reg,
					const uint8_t *data,
					uint8_t cnt,
					dynamixel_port_t port,
					dynamixel_Callback_t callback,
					bool expectsAnswer,
					transactionPriority_t priority);

bool dynamixel_reg_write(dynamixel_motorID_t addr,
					dynamixel_register_t reg,
					const uint8_t *data,
					uint8_t cnt,
					dynamixel_port_t port,
					dynamixel_Callback_t callback,
					bool expectsAnswer,
					transactionPriority_t priority);

bool dynamixel_action(dynamixel_motorID_t addr,
					dynamixel_port_t port,
					transactionPriority_t priority);

bool dynamixel_reset(dynamixel_motorID_t addr,
					dynamixel_port_t port,
					transactionPriority_t priority);

bool dynamixel_sync_write(const dynamixel_motorID_t *addr,
						uint8_t addrCnt,
						dynamixel_register_t reg,
						const uint8_t **data,
						uint8_t dataCnt,
						dynamixel_port_t port,
						transactionPriority_t priority);

#endif /* DYNAMIXEL_H_ */
