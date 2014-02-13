/*
 * float.c
 *
 *  Created on: Nov 21, 2012
 *      Author: lutz
 */


#include <flawless/init/systemInitializer.h>
#include <flawless/stdtypes.h>
#include <math.h>

#define SCB_CPACR (*(volatile uint32_t *)(0xE000ED88))

static void floating_init(void);
MODULE_INIT_FUNCTION(floating, 2, floating_init)
static void floating_init(void)
{
	/* enable floating point calculation on coprocessor */
	SCB_CPACR |= (3 << 22) | (3 << 20);
}
