/*
 * usrButtons.c
 *
 *  Created on: Mar 7, 2013
 *      Author: lutz
 */

#include <flawless/init/systemInitializer.h>
#include <flawless/timer/swTimer.h>
#include <flawless/core/msg_msgPump.h>
#include <flawless/protocol/msgProxy.h>

#include <flawless/config/msgIDs.h>

#include <libopencm3/stm32/f4/rcc.h>
#include <libopencm3/stm32/f4/gpio.h>
#include <libopencm3/stm32/f4/syscfg.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/f4/nvic.h>

#include <interfaces/usrButtons.h>
#include <target/stm32f4/gpio_interrupts.h>


#define USR_BUTTON_1_A_PORT GPIOB
#define USR_BUTTON_1_K_PORT GPIOB
#define USR_BUTTON_1_A_PIN  GPIO5
#define USR_BUTTON_1_K_PIN  GPIO4

#define USR_BUTTON_2_A_PORT GPIOB
#define USR_BUTTON_2_K_PORT GPIOB
#define USR_BUTTON_2_A_PIN  GPIO7
#define USR_BUTTON_2_K_PIN  GPIO6

#define USR_BUTTON_3_A_PORT GPIOB
#define USR_BUTTON_3_K_PORT GPIOB
#define USR_BUTTON_3_A_PIN  GPIO14
#define USR_BUTTON_3_K_PIN  GPIO9

#define USR_BUTTON_4_A_PORT GPIOB
#define USR_BUTTON_4_K_PORT GPIOB
#define USR_BUTTON_4_A_PIN  GPIO12
#define USR_BUTTON_4_K_PIN  GPIO13

#define USR_BUTTON_5_A_PORT GPIOB
#define USR_BUTTON_5_K_PORT GPIOB
#define USR_BUTTON_5_A_PIN  GPIO0
#define USR_BUTTON_5_K_PIN  GPIO1

typedef struct tag_usrButtonSetup
{
	uint32_t aPort, kPort;
	uint16_t aPin,  kPin;
} usrButtonSetup_t;

#define BUTTON_EVALUATION_TIME_INTERVAL_MS 50U

MSG_PUMP_DECLARE_MESSAGE_BUFFER_POOL(usrButton1BP, usrButtonState_t, 2, MSG_ID_USR_BUTTON_1_PRESSED)
MSG_PUMP_DECLARE_MESSAGE_BUFFER_POOL(usrButton2BP, usrButtonState_t, 2, MSG_ID_USR_BUTTON_2_PRESSED)
MSG_PUMP_DECLARE_MESSAGE_BUFFER_POOL(usrButton3BP, usrButtonState_t, 2, MSG_ID_USR_BUTTON_3_PRESSED)
MSG_PUMP_DECLARE_MESSAGE_BUFFER_POOL(usrButton4BP, usrButtonState_t, 2, MSG_ID_USR_BUTTON_4_PRESSED)


static usrButtonState_t g_buttonStates[USR_BUTTON_CNT];

static const usrButtonSetup_t g_usrButtonSetup[USR_BUTTON_CNT + 1] =
{
		{
				USR_BUTTON_1_A_PORT, USR_BUTTON_1_K_PORT,
				USR_BUTTON_1_A_PIN , USR_BUTTON_1_K_PIN
		},
		{
				USR_BUTTON_2_A_PORT, USR_BUTTON_2_K_PORT,
				USR_BUTTON_2_A_PIN , USR_BUTTON_2_K_PIN
		},
		{
				USR_BUTTON_3_A_PORT, USR_BUTTON_3_K_PORT,
				USR_BUTTON_3_A_PIN , USR_BUTTON_3_K_PIN
		},
		{
				USR_BUTTON_4_A_PORT, USR_BUTTON_4_K_PORT,
				USR_BUTTON_4_A_PIN , USR_BUTTON_4_K_PIN
		},
		{
				USR_BUTTON_5_A_PORT, USR_BUTTON_5_K_PORT,
				USR_BUTTON_5_A_PIN , USR_BUTTON_5_K_PIN
		}
};

static usrButtonConfig_t g_usrButtonConfigs[USR_BUTTON_CNT] = {USR_BUTTON_CNF_OFF};

void setupButton(usrButton_t button, usrButtonConfig_t config)
{
	if (button < USR_BUTTON_CNT)
	{
		const usrButtonSetup_t *setup = &(g_usrButtonSetup[button]);

		gpio_disable_interrupt(setup->aPort, setup->aPin);
		gpio_disable_interrupt(setup->kPort, setup->kPin);

		g_usrButtonConfigs[button] = config;

		switch (config) {
			case USR_BUTTON_CNF_OFF:
				gpio_mode_setup(setup->aPort, GPIO_MODE_INPUT, GPIO_PUPD_NONE, setup->aPin);
				gpio_mode_setup(setup->kPort, GPIO_MODE_INPUT, GPIO_PUPD_NONE, setup->kPin);
				break;
			case USR_BUTTON_CNF_LED_ON:
				gpio_mode_setup(setup->aPort, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, setup->aPin);
				gpio_mode_setup(setup->kPort, GPIO_MODE_INPUT, GPIO_PUPD_NONE, setup->kPin);
				gpio_set_output_options(setup->aPort, GPIO_OTYPE_OD, GPIO_OSPEED_100MHZ, setup->aPin);
				gpio_clear(setup->aPort, setup->aPin);
				break;
			case USR_BUTTON_CNF_LED_ON_LISTENING:
				gpio_mode_setup(setup->aPort, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, setup->aPin);
				gpio_mode_setup(setup->kPort, GPIO_MODE_INPUT, GPIO_PUPD_NONE, setup->kPin);
				gpio_set_output_options(setup->aPort, GPIO_OTYPE_OD, GPIO_OSPEED_100MHZ, setup->aPin);
				gpio_clear(setup->aPort, setup->aPin);
				gpio_enable_interrupt(setup->kPort, setup->kPin);
				break;
			case USR_BUTTON_CNF_LED_OFF_LISTENING:
				gpio_mode_setup(setup->kPort, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, setup->kPin);
				gpio_mode_setup(setup->aPort, GPIO_MODE_INPUT, GPIO_PUPD_NONE, setup->aPin);

				gpio_set_output_options(setup->kPort, GPIO_OTYPE_OD, GPIO_OSPEED_100MHZ, setup->kPin);
				gpio_clear(setup->kPort, setup->kPin);
				gpio_enable_interrupt(setup->aPort, setup->aPin);
			break;
			default:
				break;
		}
	}
}

static msgPump_MsgID_t buttonIDtoMsgID(const usrButton_t button)
{
	msgID_t ret = MSG_ID_USR_BUTTON_1_PRESSED;
	switch (button)
	{
		case USR_BUTTON_1:
			ret = MSG_ID_USR_BUTTON_1_PRESSED;
			break;
		case USR_BUTTON_2:
			ret = MSG_ID_USR_BUTTON_2_PRESSED;
			break;
		case USR_BUTTON_3:
			ret = MSG_ID_USR_BUTTON_3_PRESSED;
			break;
		case USR_BUTTON_4:
			ret = MSG_ID_USR_BUTTON_4_PRESSED;
			break;
		default:
			break;
	}
	return ret;
}

static void buttonEvaluation();
static void buttonEvaluation()
{
	uint32_t cnf_a = 0U;
	uint32_t cnf_k = 0U;
	uint8_t i = 0U;
	for (i = 0U; i < USR_BUTTON_CNT; ++i)
	{
		cnf_a |= (gpio_port_read(g_usrButtonSetup[i].aPort) & g_usrButtonSetup[i].aPin);
		cnf_k |= (gpio_port_read(g_usrButtonSetup[i].kPort) & g_usrButtonSetup[i].kPin);
	}

	for (i = 0U; i < USR_BUTTON_CNT; ++i)
	{
		usrButtonConfig_t config = g_usrButtonConfigs[i];
		usrButtonState_t state_K = (0 == (cnf_k & g_usrButtonSetup[i].kPin)) ? USR_BUTTON_PRESSED: USR_BUTTON_RELEASED;
		usrButtonState_t state_A = (0 == (cnf_a & g_usrButtonSetup[i].aPin)) ? USR_BUTTON_PRESSED: USR_BUTTON_RELEASED;
		const msgPump_MsgID_t msgID = buttonIDtoMsgID(i);

		switch (config)
		{
			case USR_BUTTON_CNF_LED_ON_LISTENING:
				if (state_K != g_buttonStates[i])
				{
					msgPump_postMessage(msgID, &state_K);
					g_buttonStates[i] = state_K;
				}
				gpio_enable_interrupt(g_usrButtonSetup[i].kPort, g_usrButtonSetup[i].kPin);
				break;
			case USR_BUTTON_CNF_LED_OFF_LISTENING:
				if (state_A != g_buttonStates[i])
				{
					msgPump_postMessage(msgID, &state_A);
					g_buttonStates[i] = state_A;
				}
				gpio_enable_interrupt(g_usrButtonSetup[i].aPort, g_usrButtonSetup[i].aPin);
				break;
			default:
				break;
		}
	}

}

void onButtonPressed(void *info)
{
	const usrButtonSetup_t* setup = (const usrButtonSetup_t*)info;

	gpio_disable_interrupt(setup->aPort, setup->aPin);
	gpio_disable_interrupt(setup->kPort, setup->kPin);

	swTimer_registerOnTimer(&buttonEvaluation, BUTTON_EVALUATION_TIME_INTERVAL_MS, true);
}

static void usrButtons_init(void);
MODULE_INIT_FUNCTION(usrButtons, 5, usrButtons_init)
static void usrButtons_init(void)
{
	uint8_t i = 0U;

	RCC_AHB1ENR |= RCC_AHB1ENR_IOPBEN;

	for (i = 0U; i < USR_BUTTON_CNT; ++i)
	{
		g_buttonStates[i] = USR_BUTTON_RELEASED;

		gpio_registerFor_interrupt(&onButtonPressed, g_usrButtonSetup[i].aPort, g_usrButtonSetup[i].aPin, GPIO_TRIGGER_LEVEL_FALLING | GPIO_TRIGGER_LEVEL_RISING, &(g_usrButtonSetup[i]));
		gpio_registerFor_interrupt(&onButtonPressed, g_usrButtonSetup[i].kPort, g_usrButtonSetup[i].kPin, GPIO_TRIGGER_LEVEL_FALLING | GPIO_TRIGGER_LEVEL_RISING, &(g_usrButtonSetup[i]));

		setupButton(i, USR_BUTTON_CNF_OFF);
	}

	msgProxy_addMsgForBroadcast(MSG_ID_USR_BUTTON_1_PRESSED);
	msgProxy_addMsgForBroadcast(MSG_ID_USR_BUTTON_2_PRESSED);
	msgProxy_addMsgForBroadcast(MSG_ID_USR_BUTTON_3_PRESSED);
	msgProxy_addMsgForBroadcast(MSG_ID_USR_BUTTON_4_PRESSED);
}

