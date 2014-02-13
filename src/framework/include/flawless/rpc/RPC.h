/*
 * RPC.h
 *
 *  Created on: Apr 24, 2012
 *      Author: lutz
 */

#ifndef RPC_H_
#define RPC_H_

#include <flawless/stdtypes.h>
#include <flawless/misc/compilerWarnings.h>
#include <flawless/rpc/RPC_Interface.h>
#include <flawless/protocol/genericFlawLessProtocol_Data.h>

typedef uint16_t RPC_paramLength_t;

typedef uint32_t rpcHandleIdentifier_t;

typedef void (*moduleRPCFunction)(const void*, void*, const RPC_paramLength_t*, RPC_paramLength_t*, const RPC_ID_t, const flawLessInterfaceDescriptor_t);


typedef struct tag_moduleRPCInfoHandle
{
	const char *const shortName;
	const char *const descriptiveName;
	const char *const in_parameterType;
	const char *const o_parameterType;
	void * const outParameterPtr;
	const RPC_paramLength_t i_parameterSize;
	const RPC_paramLength_t o_parameterSize;
	const uint8_t i_parameterCanBeSmaller;
	const uint8_t o_parameterCanBeSmaller;
	const uint8_t padding1, padding2;
	const uint32_t padding3, padding4;
	const moduleRPCFunction const rpcFunction;
}moduleRPCInfoHandle_t;

#define RPC_FUNCTION(functionName, shortName, descriptiveName, in_parameterType, o_parameterType, inParamCanBeSmaller, oParamCanBeSmaller) \
	void functionName(const void *i_param, void *o_param, const RPC_paramLength_t *i_inParamSize, RPC_paramLength_t *io_retParamSize, const RPC_ID_t i_rpcID, const flawLessInterfaceDescriptor_t i_interface); \
	__attribute__ ((unused))\
	__attribute__ ((section(".moduleRPCShortNames"))) \
	const char const functionName ##_shortName[] = shortName; \
	__attribute__ ((unused))\
	__attribute__ ((section(".moduleRPCDescriptiveNames"))) \
	const char const functionName ##_descriptiveName[] = descriptiveName; \
	__attribute__ ((unused))\
	__attribute__ ((section(".moduleRPCParameterInTypes"))) \
	const char const functionName ##_in_parameterType[] = #in_parameterType; \
	__attribute__ ((unused))\
	__attribute__ ((section(".moduleRPCParameterOutTypes"))) \
	const char const functionName ##_out_parameterType[] = #o_parameterType; \
	static o_parameterType functionName ##_o_parameterBuf; \
	__attribute__ ((unused))\
	__attribute__ ((section(".moduleRPCHandles"))) \
	const moduleRPCInfoHandle_t functionName ##_RPChandle = {functionName ## _shortName,\
																	functionName ## _descriptiveName,\
																	functionName ## _in_parameterType,\
																	functionName ## _out_parameterType,\
																	&(functionName ##_o_parameterBuf),\
																	sizeof(in_parameterType),\
																	sizeof(o_parameterType),\
																	inParamCanBeSmaller,\
																	oParamCanBeSmaller,\
																	0,0,0,0, \
																	functionName};\
	void functionName(const void *i_param, void *o_param, const RPC_paramLength_t *i_inParamSize, RPC_paramLength_t *io_retParamSize, const RPC_ID_t i_rpcID, const flawLessInterfaceDescriptor_t i_interface) \
	{ \
		UNUSED(i_param);\
		UNUSED(o_param);\
		UNUSED(i_inParamSize); \
		UNUSED(i_rpcID);\
		UNUSED(i_interface);\
		*io_retParamSize = sizeof(o_parameterType); \



void sendRPC_responseManually(RPC_ID_t, RPC_paramLength_t, const void*, const flawLessInterfaceDescriptor_t i_interfaceDescriptor);

#endif /* RPC_H_ */
