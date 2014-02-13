/*
 * systemInitializer.h
 *
 *  Created on: Apr 24, 2012
 *      Author: lutz
 */

#ifndef SYSTEMINITIALIZER_H_
#define SYSTEMINITIALIZER_H_

/*
 * how to initialize a module:
 * Use the macro MODULE_INIT_FUNCTION and fill it with the appropriate information.
 * Have a function conforming the signature of moduleInitFunc_t and pass it as function argument.
 * For the sake of having simple names for initfunctions this function must be declared static.
 * The initLevel argument describes when the module will be initialized.
 * List and meanings of the initlevels:
 * Level 0:   Initialize the Hardware1 . (clocks etc.)
 * Level 1:   Initialize the Hardware2 . (external memory etc.)
 * Level 2:   Initialize the Hardware3 . (everything else which has to work before application access etc.)
 * Level 3,4: Initialization of system components
 * Level 5-9: Initialization of applications (modules). You should use those levels for your module code
 *
 */

typedef void (*moduleInitFunc_t)(void);


#define MODULE_INIT_FUNCTION(moduleName, initLevel, function) \
	__attribute__((unused))\
	__attribute__ ((section(".moduleInitializeFunction" #initLevel))) \
	const moduleInitFunc_t moduleName ## _initFuncPtr = &function; \

/*
 * this is the designated initialize function.
 * Call this once the application starts (usually in the main function)
 * This function performs the initialization of all modules in the order of the init level.
 * This function must be called before entering the message pump main loop!
 */
void systemInitialize(void);

#endif /* SYSTEMINITIALIZER_H_ */
