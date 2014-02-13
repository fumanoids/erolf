/*
 * main.c
 *
 *  Created on: Oct 17, 2012
 *      Author: lutz
 */

#include <flawless/init/systemInitializer.h>

#include <flawless/core/msg_msgPump.h>

#include <flawless/logging/logging.h>

int main(void)
{
	systemInitialize();

	LOG_VERBOSE_0("all intialized");

	msgPump_pumpMessage();

	return 0;
}
