/*
 * msgProxy.h
 *
 *  Created on: 09.09.2012
 *      Author: lutz
 */

#ifndef MSGPROXY_H_
#define MSGPROXY_H_

#include <flawless/config/msgIDs.h>

#define GENERIC_PROTOCOL_MSG_PROXY_ENDPOINT_ID 4U

void msgProxy_addMsgForBroadcast(msgID_t msgID);
void msgProxy_rmMsgForBroadcast(msgID_t msgID);

#endif /* MSGPROXY_H_ */
