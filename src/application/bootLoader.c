/*
 * test.c
 *
 *  Created on: Oct 23, 2012
 *      Author: lutz
 */

#include <flawless/init/systemInitializer.h>
#include <flawless/rpc/RPC.h>
#include <flawless/timer/swTimer.h>

#include <libopencm3/stm32/f4/gpio.h>
#include <libopencm3/stm32/f4/rcc.h>
#include <libopencm3/stm32/f4/scb.h>
#include <libopencm3/stm32/wwdg.h>


#define BOOT_CAP_CHARGE_TIME_MS 100

static void timerCB(void);
static void timerCB(void)
{
	/* perform system reset */
	SCB_AIRCR = SCB_AIRCR_VECTKEY | SCB_AIRCR_SYSRESETREQ;
}

RPC_FUNCTION(enterBootLoader, "enterBootLoader", "performs a reset of the processor into the embedded bootloader", uint8_t, uint16_t, true, true)
	/* charge the boot capacitor */
	gpio_set(GPIOB, GPIO8);

	swTimer_registerOnTimer(&timerCB, BOOT_CAP_CHARGE_TIME_MS, true);
	*((uint16_t*)o_param) = BOOT_CAP_CHARGE_TIME_MS;
}

static void booloader_init(void);
MODULE_INIT_FUNCTION(bootloader, 9, booloader_init);
static void booloader_init(void)
{
	RCC_AHB1ENR |= RCC_AHB1ENR_IOPBEN;
	gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO8);
	gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, GPIO8);
}
