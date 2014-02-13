/*
 * systemMsgIDs.h
 *
 *  Created on: Jun 29, 2012
 *      Author: lutz
 */

#ifndef SYSTEMMSGIDS_H_
#define SYSTEMMSGIDS_H_

typedef enum tag_systemMsgIDs
{
	MSG_ID_GENERIC_PROTOCOL_PACKET_READY = -1,
	MSG_ID_SYSTEM_RPC_DISPATCHER = -2,
	MSG_ID_GENERIC_PROTOCOL_PACKET_STATISTICS = -3,
	MSG_ID_SYSTEM_FIRST_ID = -3, /* the min value of the system ids */
	MSG_ID_SYSTEM_MSG_COUNT = 3
} systemMsgID_t;



#endif /* SYSTEMMSGIDS_H_ */
