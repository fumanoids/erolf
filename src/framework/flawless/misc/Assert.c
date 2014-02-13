/*
 * MoBoAssert.c
 *
 *  Created on: 22.11.2011
 *      Author: lutz
 */

#include <string.h>

#include <flawless/misc/Assert.h>

#define LOG_LEVEL 4
#include <flawless/logging/logging.h>

void assert_failFunction(const char* fileName, const char* expr, const uint16_t lineNo)
{
	UNUSED(fileName);
	UNUSED(lineNo);
	UNUSED(expr);
	// blocking
	LOG_ERROR_0("ASSERT FAILED");

	while (true)
	{
	}
}
