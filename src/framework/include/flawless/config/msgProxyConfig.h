/*
 * msgProxy.h
 *
 *  Created on: Jan 2, 2013
 *      Author: lutz
 */

#ifndef MSGPROXY_CONFIG_H_
#define MSGPROXY_CONFIG_H_


#include "msgIDs.h"


/*
 * a map from external messages (msgIDs of messages transmitted on external busses)
 * to internal message IDs
 */

typedef struct tag_extIDtoIntIDMap
{
	uint16_t externalID;
	msgID_t internalID;
} extIDtoIntIDPair_t;

#define MSG_PROXY_EXT_2_INT_MSG_ID_MAP { \
		{1U, MSG_ID_BAT_VOLTAGE},\
		{2U, MSG_ID_REL_BAT_VOLTAGE},\
		{3U, MSG_ID_USER_MOTOR_SWITCH},\
		{4U, MSG_ID_USR_BUTTON_1_PRESSED},\
		{5U, MSG_ID_USR_BUTTON_2_PRESSED},\
		{6U, MSG_ID_USR_BUTTON_3_PRESSED},\
		{7U, MSG_ID_USR_BUTTON_4_PRESSED},\
		{8U, MSG_ID_MOTOR_PWR_FAILURE}\
	}


#endif /* MSGPROXY_CONFIG_H_ */
