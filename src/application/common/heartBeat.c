/*
 * heartBeat.c
 *
 *  Created on: March 06, 2013
 *      Author: Simon G. G.
 */

#include <libopencm3/stm32/f4/gpio.h>
#include <libopencm3/stm32/f4/rcc.h>
#include <flawless/core/msg_msgPump.h>
#include <flawless/config/msgIDs.h>

#include <flawless/init/systemInitializer.h>
#include <flawless/timer/swTimer.h>
#include <interfaces/systemTime.h>
#include "heartBeat.h"

#define LED_PORT GPIOC
#define LED_PIN GPIO3

MSG_PUMP_DECLARE_MESSAGE_BUFFER_POOL(heartBeat_buffer, int, 2, MSG_ID_HEARTBEAT);


static int heartBeatActive = 1;

void setHeartBeat(int active)
{
	heartBeatActive = active;
}

int getHeartBeat(void)
{
	return heartBeatActive;
}

void setHeartBeatLED(int active)
{
	if (active)
		gpio_clear(LED_PORT, LED_PIN);
	else
		gpio_set(LED_PORT, LED_PIN);

}


static void heartBeat_timerCB(void);
static void heartBeat_timerCB(void) {
	if (getHeartBeat()) {

		uint64_t phase = (getCurrentTime() / 80ULL)%15;
		int x = 0;

		// Is Timer Synchronized?
		uint8_t sync = timeIsSync();
		if (sync == 0) {
			phase = (getSystemTimeUS() / 80000ULL)%15;
		}

		if (phase == 0 || (phase == 3 && sync != 0))
			x = 1;
	
		msgPump_postMessage(MSG_ID_HEARTBEAT, &x);
	}
}

static void heartBeat_msgPumpCallback(msgPump_MsgID_t id, const void *buffer);
static void heartBeat_msgPumpCallback(msgPump_MsgID_t id, const void *buffer) {
	int ledStatus = *((int const*)buffer);

	setHeartBeatLED(ledStatus);

	UNUSED(id);
}

static void heartBeat_init(void);
MODULE_INIT_FUNCTION(heartBeat, 9, heartBeat_init);
static void heartBeat_init(void) {

	RCC_AHB1ENR |= RCC_AHB1ENR_IOPCEN;

	gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN);
	gpio_set_output_options(LED_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, LED_PIN);

	swTimer_registerOnTimer(&heartBeat_timerCB, 10, false);

	const bool registerSuccess2 = msgPump_registerOnMessage(MSG_ID_HEARTBEAT,  &heartBeat_msgPumpCallback);
	ASSERT(false != registerSuccess2);
}
