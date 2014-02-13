/*
 * configuration.h
 *
 *  Created on: Mar 28, 2013
 *      Author: lutz
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <flawless/stdtypes.h>

/* this goes to the flash and will be flashed each time new software goes on the uC*/
typedef struct tag_configVariableDescriptor
{
	const char *variableName;        /* name of the variable */
	const char *fileName;            /* file where the variable was defined (must be unique in combination with the variable) */
	void *dataPtr;             /* pointer to the configuration data in the RAM */
	const void *defaultValuesRegion; /* storage of default values of this configuration */
	uint16_t dataLen;          /* amount of data of this configuration data */
} configVariableDescriptor_t;


#define CONFIG_VARIABLE(type, name, defaultValueArea) \
	__attribute__ ((section(".applicationConfigRAM"))) \
	static type name; \
	static const char name ##_variableName[] = #name; \
	static const char name ##_fileName[] = __FILE__; \
	__attribute__((unused))\
	__attribute__ ((section(".applicationConfigDescriptors"))) \
	const configVariableDescriptor_t name ##_descriptor = \
					{ \
						name##_variableName, \
						name##_fileName, \
						&name, \
						defaultValueArea, \
						sizeof(name), \
					};


void config_updateToFlash();


#endif /* CONFIGURATION_H_ */
