/*
 * usrButtons.h
 *
 *  Created on: Mar 30, 2013
 *      Author: lutz
 */

#ifndef USRBUTTONS_H_
#define USRBUTTONS_H_

typedef enum tag_userButton
{
	USR_BUTTON_1,
	USR_BUTTON_2,
	USR_BUTTON_3,
	USR_BUTTON_4,
	USR_BUTTON_CNT = 4
} usrButton_t;

typedef enum tag_userButtonState
{
	USR_BUTTON_RELEASED = 0U,
	USR_BUTTON_PRESSED = 1U
} usrButtonState_t;

typedef enum tag_userButtonConfig
{
	USR_BUTTON_CNF_OFF = 0,
	USR_BUTTON_CNF_LED_ON = 1,
	USR_BUTTON_CNF_LED_ON_LISTENING = 2,
	USR_BUTTON_CNF_LED_OFF_LISTENING = 3
} usrButtonConfig_t;


void setupButton(usrButton_t button, usrButtonConfig_t config);

#endif /* USRBUTTONS_H_ */
