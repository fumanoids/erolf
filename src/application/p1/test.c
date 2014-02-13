/*
 * test.c
 *
 *  Created on: Oct 25, 2012
 *      Author: lutz
 */


#include <flawless/init/systemInitializer.h>
#include <flawless/timer/swTimer.h>
#include <flawless/core/msg_msgPump.h>
#include <flawless/config/msgIDs.h>

#include <interfaces/usrButtons.h>

static void onButton(msgPump_MsgID_t msgID, const void *ptr);
static void onButton(msgPump_MsgID_t msgID, const void *ptr)
{
	static usrButton_t thebutton = USR_BUTTON_1;
	const usrButtonState_t *state = (const usrButtonState_t*)ptr;

	UNUSED(msgID);

	if (USR_BUTTON_RELEASED == *state)
	{
		for (usrButton_t button = USR_BUTTON_1; button < USR_BUTTON_CNT; ++button)
		{
			setupButton(button, USR_BUTTON_CNF_OFF);
		}

		setupButton(thebutton, USR_BUTTON_CNF_LED_ON_LISTENING);

		thebutton += 1;
		if (USR_BUTTON_CNT <= thebutton)
		{
			thebutton = USR_BUTTON_1;
		}
	}

}

static void test_init(void);
MODULE_INIT_FUNCTION(test,NEVER, test_init);
static void test_init(void)
{
	msgPump_registerOnMessage(MSG_ID_USR_BUTTON_1_PRESSED, &onButton);
	msgPump_registerOnMessage(MSG_ID_USR_BUTTON_2_PRESSED, &onButton);
	msgPump_registerOnMessage(MSG_ID_USR_BUTTON_3_PRESSED, &onButton);
	msgPump_registerOnMessage(MSG_ID_USR_BUTTON_4_PRESSED, &onButton);
}

