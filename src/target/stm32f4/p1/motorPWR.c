/*
 * motorPWR.
 *
 *  Created on: 21.11.2013
 *      Author: lutz
 */

#include <flawless/init/systemInitializer.h>
#include <flawless/timer/swTimer.h>
#include <flawless/core/msg_msgPump.h>
#include <flawless/config/msgIDs.h>
#include <flawless/protocol/msgProxy.h>
#include <interfaces/systemTime.h>

#include <target/stm32f4/gpio_interrupts.h>

#include <libopencm3/stm32/f4/syscfg.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/f4/gpio.h>
#include <libopencm3/stm32/f4/rcc.h>

#define DYNAMIXEL_POWER_RESET_CLK_PORT GPIOC
#define DYNAMIXEL_POWER_RESET_CLK_PIN GPIO14
#define DYNAMIXEL_POWER_RESET_D_PORT GPIOC
#define DYNAMIXEL_POWER_RESET_D_PIN GPIO13

#define DYNAMIXEL_POWER_BUS_OFF_PORT GPIOB
#define DYNAMIXEL_POWER_BUS_OFF_PIN GPIO15

#define COLD_START_INIT_DELAY_MS 500

#define RESTART_MOTORS_DELAY_AFTER_FAILURE_MS 500

MSG_PUMP_DECLARE_MESSAGE_BUFFER_POOL(motorPWRFailureBP, uint8_t, 2, MSG_ID_MOTOR_PWR_FAILURE)

static void onReleasePWRLine(void)
{
//	gpio_mode_setup(DYNAMIXEL_POWER_BUS_OFF_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, DYNAMIXEL_POWER_BUS_OFF_PIN);
}

static void reenableMotors()
{
	gpio_mode_setup(DYNAMIXEL_POWER_BUS_OFF_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, DYNAMIXEL_POWER_BUS_OFF_PIN);
	gpio_set_output_options(DYNAMIXEL_POWER_BUS_OFF_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, DYNAMIXEL_POWER_BUS_OFF_PIN);
	gpio_set(DYNAMIXEL_POWER_BUS_OFF_PORT, DYNAMIXEL_POWER_BUS_OFF_PIN);

	gpio_set(DYNAMIXEL_POWER_RESET_CLK_PORT, DYNAMIXEL_POWER_RESET_CLK_PIN);
	gpio_clear(DYNAMIXEL_POWER_RESET_CLK_PORT, DYNAMIXEL_POWER_RESET_CLK_PIN);

	swTimer_registerOnTimer(&onReleasePWRLine, COLD_START_INIT_DELAY_MS, true);

}

static void onMtrPwrShutdown(void *info)
{
	UNUSED(info);
	swTimer_registerOnTimer(&reenableMotors, RESTART_MOTORS_DELAY_AFTER_FAILURE_MS, TRUE);
	uint8_t bla = 1;
	msgPump_postMessage(MSG_ID_MOTOR_PWR_FAILURE, &bla);
}

static void dynamixelPower_init(void);
MODULE_INIT_FUNCTION(dynamixelPower, 9, dynamixelPower_init)
static void dynamixelPower_init(void)
{
	RCC_AHB1ENR |= RCC_AHB1ENR_IOPBEN;
	RCC_AHB1ENR |= RCC_AHB1ENR_IOPCEN;

	gpio_mode_setup(DYNAMIXEL_POWER_RESET_CLK_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, DYNAMIXEL_POWER_RESET_CLK_PIN);
	gpio_set_output_options(DYNAMIXEL_POWER_RESET_CLK_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, DYNAMIXEL_POWER_RESET_CLK_PIN);

	gpio_mode_setup(DYNAMIXEL_POWER_RESET_D_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, DYNAMIXEL_POWER_RESET_D_PIN);
	gpio_set_output_options(DYNAMIXEL_POWER_RESET_D_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, DYNAMIXEL_POWER_RESET_D_PIN);

	gpio_set(DYNAMIXEL_POWER_RESET_D_PORT, DYNAMIXEL_POWER_RESET_D_PIN);

	gpio_mode_setup(DYNAMIXEL_POWER_BUS_OFF_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, DYNAMIXEL_POWER_BUS_OFF_PIN);
	gpio_set_output_options(DYNAMIXEL_POWER_BUS_OFF_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, DYNAMIXEL_POWER_BUS_OFF_PIN);
	gpio_set(DYNAMIXEL_POWER_BUS_OFF_PORT, DYNAMIXEL_POWER_BUS_OFF_PIN);

	gpio_set(DYNAMIXEL_POWER_RESET_CLK_PORT, DYNAMIXEL_POWER_RESET_CLK_PIN);
	gpio_clear(DYNAMIXEL_POWER_RESET_CLK_PORT, DYNAMIXEL_POWER_RESET_CLK_PIN);

	gpio_registerFor_interrupt(&onMtrPwrShutdown, DYNAMIXEL_POWER_BUS_OFF_PORT, DYNAMIXEL_POWER_BUS_OFF_PIN, GPIO_TRIGGER_LEVEL_FALLING, NULL);
	gpio_enable_interrupt(DYNAMIXEL_POWER_BUS_OFF_PORT, DYNAMIXEL_POWER_BUS_OFF_PIN);

	msgProxy_addMsgForBroadcast(MSG_ID_MOTOR_PWR_FAILURE);
	reenableMotors();
}
