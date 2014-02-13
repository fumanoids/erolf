/*
 * memory.c
 *
 *  Created on: Nov 21, 2012
 *      Author: lutz
 */

#include <flawless/init/systemInitializer.h>

/* Symbols exported by the linker script(s): */
extern unsigned _data_loadaddr, _data, _edata, _ebss, _ramBegin;

static void memory_init(void);
MODULE_INIT_FUNCTION(memory, 1, memory_init)
static void memory_init(void)
{

	volatile unsigned *src, *dest;
	volatile unsigned blubb;
	for (dest = &_ramBegin; dest < &blubb; ++dest)
	{
		*dest = 0U;
	}
	for (src = &_data_loadaddr, dest = &_data; dest < &_edata; src++, dest++)
	{
		*dest = *src;
	}

	while (dest < &_ebss)
	{
		*dest++ = 0;
	}
}

