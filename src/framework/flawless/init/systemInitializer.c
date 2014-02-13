/*
 * systemInitializer.c
 *
 *  Created on: Apr 24, 2012
 *      Author: lutz
 */

#include <flawless/init/systemInitializer.h>
#include <flawless/stdtypes.h>

extern const moduleInitFunc_t _initFunctions_Begin;
extern const moduleInitFunc_t _initFunctions_End;


void systemInitialize(void)
{
	const moduleInitFunc_t *funcPtr = &_initFunctions_Begin;
	while (&_initFunctions_End > funcPtr)
	{
		if (NULL != *funcPtr)
		{
			(void)(*(funcPtr))();
		}
		++funcPtr;
	}
}


