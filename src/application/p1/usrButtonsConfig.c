/*
 * usrButtons.c
 *
 *  Created on: Apr 26, 2013
 *      Author: lutz
 */

#include <flawless/init/systemInitializer.h>
#include <flawless/core/msg_msgPump.h>
#include <flawless/config/msgIDs.h>
#include <string.h>

#include "interfaces/usrButtons.h"

#include <flawless/rpc/RPC.h>

typedef struct tag_usrButtonConfigStruct
{
	uint8_t button;
	uint8_t usrButtonConfig;
} usrButtonConfigStruct_t;


typedef uint8_t buttonPressedCount_t[USR_BUTTON_CNT];

static buttonPressedCount_t g_buttonPressedCounts = {0U};

RPC_FUNCTION(enableButton, "enableUsrButton", "setup a user button (first byte which button, second byte configuration)", usrButtonConfigStruct_t, uint8_t, false, false)
	const usrButtonConfigStruct_t* i_data = (const usrButtonConfigStruct_t*) i_param;

	setupButton(i_data->button, i_data->usrButtonConfig);

	*((uint8_t*)o_param) = 1;
}

RPC_FUNCTION(getButtonsPressedCnt, "getButtonsPressedCnt", "get the amount of press events for all buttons (one byte per button) the counts will not be cleared!", uint8_t, buttonPressedCount_t, true, false)
	memcpy(o_param, &g_buttonPressedCounts, sizeof(g_buttonPressedCounts));
}

RPC_FUNCTION(resetButtonCnt, "resetButtonCnt", "reset the amount of press events for all buttons", uint8_t, uint8_t, true, true)
	uint8_t i;
	for (i = 0U; i < sizeof(g_buttonPressedCounts); ++i)
	{
		g_buttonPressedCounts[i] = 0U;
	}
}

static void onButtonPressed(msgPump_MsgID_t msgID, const void *buf);
static void onButtonPressed(msgPump_MsgID_t msgID, const void *buf)
{
	const usrButtonState_t *state = (const usrButtonState_t*) buf;

	if (USR_BUTTON_PRESSED == *state)
	{
		switch (msgID)
		{
			case MSG_ID_USR_BUTTON_1_PRESSED:
				++g_buttonPressedCounts[USR_BUTTON_1];
				break;
			case MSG_ID_USR_BUTTON_2_PRESSED:
				++g_buttonPressedCounts[USR_BUTTON_2];
				break;
			case MSG_ID_USR_BUTTON_3_PRESSED:
				++g_buttonPressedCounts[USR_BUTTON_3];
				break;
			case MSG_ID_USR_BUTTON_4_PRESSED:
				++g_buttonPressedCounts[USR_BUTTON_4];
				break;
		}
	}
}

static void usrButtons_init(void);
MODULE_INIT_FUNCTION(usrButton_application, 8, usrButtons_init);
static void usrButtons_init(void)
{
	uint8_t i;
	setupButton(USR_BUTTON_2, USR_BUTTON_CNF_LED_ON_LISTENING);
	setupButton(USR_BUTTON_3, USR_BUTTON_CNF_LED_ON_LISTENING);
	setupButton(USR_BUTTON_4, USR_BUTTON_CNF_LED_ON_LISTENING);

	for (i = 0U; i < sizeof(g_buttonPressedCounts); ++i)
	{
		g_buttonPressedCounts[i] = 0U;
	}

	msgPump_registerOnMessage(MSG_ID_USR_BUTTON_1_PRESSED, &onButtonPressed);
	msgPump_registerOnMessage(MSG_ID_USR_BUTTON_2_PRESSED, &onButtonPressed);
	msgPump_registerOnMessage(MSG_ID_USR_BUTTON_3_PRESSED, &onButtonPressed);
	msgPump_registerOnMessage(MSG_ID_USR_BUTTON_4_PRESSED, &onButtonPressed);
}

