/*
 * random.c
 *
 *  Created on: Jun 13, 2013
 *      Author: lutz
 */

#include <flawless/init/systemInitializer.h>
#include <flawless/stdtypes.h>


#include <libopencm3/stm32/f4/rnd.h>
#include <libopencm3/stm32/f4/rcc.h>


uint32_t getRandomNumber()
{
	return RNG_DR;
}

static void random_init(void);
MODULE_INIT_FUNCTION(random, 3, random_init)
static void random_init(void)
{
	RCC_AHB2ENR |= RCC_AHB2ENR_RNGEN;

	RNG_CR |= RNG_CR_RNGEN;
}
