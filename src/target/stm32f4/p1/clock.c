/*
 * clock.c
 *
 *  Created on: Oct 23, 2012
 *      Author: lutz
 */


#include <flawless/init/systemInitializer.h>

#include <libopencm3/stm32/f4/rcc.h>
#include <libopencm3/stm32/f4/gpio.h>
#include <libopencm3/stm32/f4/flash.h>

#define CLOCK_MCO_PORT GPIOA
#define CLOCK_MCO_PIN  GPIO8

static void clock_init(void);
MODULE_INIT_FUNCTION(clock, 0, clock_init)
static void clock_init(void)
{

	/* intially HSI is selected as clock source */

	RCC_AHB1ENR |= RCC_AHB1ENR_IOPAEN;
	gpio_mode_setup(CLOCK_MCO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, CLOCK_MCO_PIN);
	gpio_set_output_options(CLOCK_MCO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, CLOCK_MCO_PIN);
	gpio_set_af(CLOCK_MCO_PORT, GPIO_AF0, CLOCK_MCO_PIN);

	/* mco division factor to 4 => 2MHz output frequency */
	RCC_CFGR = (RCC_CFGR & ~(0x7 << RCC_CFGR_MCO1PRE_SHIFT)) | (6 << RCC_CFGR_MCO1PRE_SHIFT);
	RCC_CFGR = (RCC_CFGR & ~(0x3 << RCC_CFGR_MCO1_SHIFT)) | (RCC_CFGR_MCO1_HSE << RCC_CFGR_MCO1_SHIFT);

	/* switch on external clock */
	RCC_CR |= RCC_CR_HSEON;

	/* wait for HSE to become ready */
	while (0 == (RCC_CR & RCC_CR_HSERDY));

	/* speed things up from here so set HSE as clock source */
	RCC_CFGR = (RCC_CFGR & ~3) | RCC_CFGR_SWS_HSE;

	/* setup RTC clock from HSE and prescaler 8 */
	RCC_CFGR = (RCC_CFGR & ~(0x1f << RCC_CFGR_RTCPRE_SHIFT)) | (8 << RCC_CFGR_RTCPRE_SHIFT);

	/* setup PLL */
	/* select HSE as PLL source */
	RCC_PLLCFGR |= RCC_PLLCFGR_PLLSRC;

	/* set PLL entry divider to 4 => 2MHz VCO entry clock */
	RCC_PLLCFGR = (RCC_PLLCFGR & ~(0x3f << RCC_PLLCFGR_PLLM_SHIFT)) | (4 << RCC_PLLCFGR_PLLM_SHIFT);

	/* set main PLL to 168 to have VCO=336MHz resulting frequency */
	RCC_PLLCFGR = (RCC_PLLCFGR & ~(0x1ff << RCC_PLLCFGR_PLLN_SHIFT)) | (168 << RCC_PLLCFGR_PLLN_SHIFT);

	/* set division factor to 2 at PLL Output */
	RCC_PLLCFGR = (RCC_PLLCFGR & ~(0x3 << RCC_PLLCFGR_PLLP_SHIFT)) | (0 << RCC_PLLCFGR_PLLP_SHIFT);

	/* set division factor to 7 for 48MHz at USB_OTG, SDIo and RNG */
	RCC_PLLCFGR = (RCC_PLLCFGR & ~(0x0f << RCC_PLLCFGR_PLLQ_SHIFT)) | (7 << RCC_PLLCFGR_PLLQ_SHIFT);



	/* enable PLL */
	RCC_CR |= RCC_CR_PLLON;
	/* wait for PLL to become ready */
	while (0 == (RCC_CR & RCC_CR_PLLRDY));

	/* setup clocks for internal busses */
	RCC_CFGR = (RCC_CFGR & ~(0x0f << RCC_CFGR_HPRE_SHIFT))  | (0 << RCC_CFGR_HPRE_SHIFT);
	RCC_CFGR = (RCC_CFGR & ~(0x07 << RCC_CFGR_PPRE1_SHIFT)) | (5 << RCC_CFGR_PPRE1_SHIFT);
	RCC_CFGR = (RCC_CFGR & ~(0x07 << RCC_CFGR_PPRE2_SHIFT)) | (4 << RCC_CFGR_PPRE2_SHIFT);

	/* set flash for 2 waitstates */
	FLASH_ACR = (FLASH_ACR & ~(0x7)) | 5;
	while ((FLASH_ACR & 0x7) != 5);

	FLASH_ACR |= FLASH_PRFTEN;// | FLASH_DCE | FLASH_ICE;

	/* switch to PLL as systen clock source */
	RCC_CFGR = (RCC_CFGR & ~3) | RCC_CFGR_SWS_PLL;
}

