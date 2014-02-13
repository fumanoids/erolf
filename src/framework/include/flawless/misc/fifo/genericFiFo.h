/*
 * genericFiFo.c
 *
 *  Created on: Jun 15, 2012
 *      Author: lutz
 */


#include <stdint.h>
#include <stddef.h>

#include <string.h>


#ifdef CREATE_GENERIC_FIFO
#undef CREATE_GENERIC_FIFO
#endif
#ifdef CREATE_FIFO_ERRORS
#undef CREATE_FIFO_ERRORS
#endif
#ifdef CREATE_FIFO_HANDLE
#undef CREATE_FIFO_HANDLE
#endif
#ifdef CREATE_FIFO_PUT_FCT
#undef CREATE_FIFO_PUT_FCT
#endif
#ifdef CREATE_FIFO_PEEK_FCT
#undef CREATE_FIFO_PEEK_FCT
#endif
#ifdef CREATE_FIFO_GET_FCT
#undef CREATE_FIFO_GET_FCT
#endif
#ifdef CREATE_FIFO_INIT_FCT
#undef CREATE_FIFO_INIT_FCT
#endif
#ifdef CREATE_FIFO_GET_CNT_FCT
#undef CREATE_FIFO_GET_CNT_FCT
#endif


#ifdef GEN_FIFO_USE_ATOMIC
#ifndef GEN_FIFO_CLI_FUNCTION
#error "For atomic use of fifo the clear interrupt enable bit function must be specified!"
#endif
#ifndef GEN_FIFO_SEI_FUNCTION
#error "For atomic use of fifo the set interrupt enable bit function must be specified!"
#endif
#else
#ifdef GEN_FIFO_CLI_FUNCTION
#undef GEN_FIFO_CLI_FUNCTION
#endif
#define GEN_FIFO_CLI_FUNCTION

#ifdef GEN_FIFO_SEI_FUNCTION
#undef GEN_FIFO_SEI_FUNCTION
#endif
#define GEN_FIFO_SEI_FUNCTION
#endif

#define CREATE_FIFO_ERRORS(fifoPrefix) \
	typedef enum tag_ ## fifoPrefix ## ErrorType \
	{\
		fifoPrefix ## _FIFO_OKAY,\
		fifoPrefix ## _FIFO_UNDERRUN,\
		fifoPrefix ## _FIFO_OVERRUN, \
		fifoPrefix ## _FIFO_INVALID_USAGE \
	} fifoPrefix ## _ErrorType_t;\


#define CREATE_FIFO_HANDLE(fifoType, fifoPrefix, fifoCnt) \
	typedef struct tag_ ## fifoPrefix ## _fifoHandle_t \
	{\
		fifoType data[fifoCnt];\
		volatile uint16_t count, start, end;\
	} fifoPrefix ##_fifoHandle_t; \



#define CREATE_FIFO_PUT_FCT(fifoType, fifoPrefix, fifoCnt) \
	static fifoPrefix ## _ErrorType_t fifoPrefix ##_put(const fifoType *i_data, fifoPrefix ##_fifoHandle_t *i_handle); \
	static fifoPrefix ## _ErrorType_t fifoPrefix ##_put(const fifoType *i_data, fifoPrefix ##_fifoHandle_t *i_handle) \
	{\
		fifoPrefix ## _ErrorType_t error = fifoPrefix ## _FIFO_OVERRUN; \
		if (NULL == i_data || NULL == i_handle) \
		{ \
			error = fifoPrefix ## _FIFO_INVALID_USAGE; \
		} else \
		{ \
			GEN_FIFO_CLI_FUNCTION; \
			if (NULL != i_handle && i_handle->count < fifoCnt) \
			{ \
				memcpy(&i_handle->data[i_handle->end], i_data, sizeof(fifoType)); \
				i_handle->end = (i_handle->end + 1U) % (fifoCnt); \
				++(i_handle->count); \
				error = fifoPrefix ## _FIFO_OKAY; \
			} \
			GEN_FIFO_SEI_FUNCTION; \
		} \
		return error; \
	}\


#define CREATE_FIFO_PEEK_FCT(fifoType, fifoPrefix, fifoCnt) \
	static fifoPrefix ## _ErrorType_t fifoPrefix ##_peek(const fifoPrefix ##_fifoHandle_t *i_handle,const  uint8_t i_index, fifoType *o_data); \
	static fifoPrefix ## _ErrorType_t fifoPrefix ##_peek(const fifoPrefix ##_fifoHandle_t *i_handle,const  uint8_t i_index, fifoType *o_data) \
	{\
		fifoPrefix ## _ErrorType_t error = fifoPrefix ## _FIFO_UNDERRUN;\
		if (NULL == o_data || NULL == i_handle || i_index >= fifoCnt) \
		{ \
			error = fifoPrefix ## _FIFO_INVALID_USAGE; \
		} else \
		{ \
			GEN_FIFO_CLI_FUNCTION;\
			if (i_handle->count > i_index)\
			{\
				const uint8_t index = (i_handle->start + i_index) % fifoCnt;\
				memcpy(o_data, &i_handle->data[index], sizeof(fifoType)); \
				error = fifoPrefix ## _FIFO_OKAY;\
			}\
			GEN_FIFO_SEI_FUNCTION;\
		} \
		return error;\
	}\


#define CREATE_FIFO_GET_FCT(fifoType, fifoPrefix, fifoCnt) \
	static fifoPrefix ## _ErrorType_t fifoPrefix ##_get(fifoPrefix ##_fifoHandle_t *i_handle, fifoType *o_data) \
	{ \
		fifoPrefix ## _ErrorType_t error = fifoPrefix ## _FIFO_UNDERRUN; \
		if (NULL == o_data || NULL == i_handle) \
		{ \
			error = fifoPrefix ## _FIFO_INVALID_USAGE; \
		} else \
		{ \
			GEN_FIFO_CLI_FUNCTION; \
			if (NULL != i_handle && i_handle->count > 0U) \
			{ \
				memcpy(o_data, &i_handle->data[i_handle->start], sizeof(fifoType)); \
				i_handle->start = (i_handle->start + 1) % fifoCnt; \
				--(i_handle->count); \
				error = fifoPrefix ## _FIFO_OKAY; \
			} \
			GEN_FIFO_SEI_FUNCTION; \
		} \
		return error; \
	}

#define CREATE_FIFO_GET_CNT_FCT(fifoType, fifoPrefix, fifoCnt) \
	static uint8_t fifoPrefix ##_getCount(const fifoPrefix ##_fifoHandle_t *i_handle); \
	static uint8_t fifoPrefix ##_getCount(const fifoPrefix ##_fifoHandle_t *i_handle) \
	{\
		uint8_t ret = 0U;\
		GEN_FIFO_CLI_FUNCTION;\
		if (NULL != i_handle)\
		{\
			ret = i_handle->count;\
		}\
		GEN_FIFO_SEI_FUNCTION;\
		return ret;\
	}


#define CREATE_FIFO_INIT_FCT(fifoType, fifoPrefix, fifoCnt) \
	static void fifoPrefix ##_init(fifoPrefix ##_fifoHandle_t *i_handle) \
	{\
		if (NULL != i_handle)\
		{\
			GEN_FIFO_CLI_FUNCTION;\
			i_handle->count = 0U;\
			i_handle->start = 0U;\
			i_handle->end = 0U;\
			GEN_FIFO_SEI_FUNCTION;\
		}\
	}


#define CREATE_GENERIC_FIFO(FIFO_TYPE, FIFO_PREFIX, FIFO_SIZE)\
	CREATE_FIFO_ERRORS(FIFO_PREFIX)\
	CREATE_FIFO_HANDLE(FIFO_TYPE, FIFO_PREFIX, FIFO_SIZE) \
	CREATE_FIFO_PUT_FCT(FIFO_TYPE, FIFO_PREFIX, FIFO_SIZE) \
	CREATE_FIFO_PEEK_FCT(FIFO_TYPE, FIFO_PREFIX, FIFO_SIZE) \
	CREATE_FIFO_GET_FCT(FIFO_TYPE, FIFO_PREFIX, FIFO_SIZE) \
	CREATE_FIFO_INIT_FCT(FIFO_TYPE, FIFO_PREFIX, FIFO_SIZE) \
	CREATE_FIFO_GET_CNT_FCT(FIFO_TYPE, FIFO_PREFIX, FIFO_SIZE)

