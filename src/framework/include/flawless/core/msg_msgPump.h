/*
 * msg_msgPump.h
 *
 *  Created on: 26.09.2011
 *      Author: lutz
 */

#ifndef MSG_MSGPUMP_H_
#define MSG_MSGPUMP_H_

#include <flawless/stdtypes.h>
#include <flawless/misc/Assert.h>
#include <flawless/config/msgPump_config.h>


/**
 * define used to declare and statically allocate memory for a message queue including it's needed memory and overhead
 */

typedef uint8_t msgBufPos_t;	  /* the number of a message in the buffer (not the index!) */
typedef uint8_t msgBufRefCount_t; /* the reference count of one message*/
typedef uint16_t msgBufMsgSize_t; /* the size of an actual message in bytes */

typedef uint8_t msgBufBufData_t; /* the type of the elements in the buffer */

typedef uint8_t msgPump_callbackCount_t;
typedef int16_t msgPump_MsgID_t;

/**
 * typedef for a callback function for the message queue
 * the function gets the msgID the data of the current message (a cast is necessary inside the callback function) and the info which was specified during the registration
 */
typedef void (*msgPump_callbackFunction_t)(msgPump_MsgID_t, const void *);


typedef enum en_msgPump_MessageBufferFlag
{
	MsgPump_MsgBufFlagNothing = 0,
	MsgPump_MsgBufFlagPostedNotDispatched = 1 << 0,
	MsgPump_MsgBufFlagInUse = 1 << 1,

	/* this can happen if some software does not use the loc/unlock mechanisms correctly. A buffer element marked with this will not be usable anymore */
	MsgPump_MsgBufFlagBufferNotUsable = 1 << 2
}msgPump_MessageBufferFlag_t;

typedef struct st_msgPumpMsgHeader
{
#ifdef MSG_PUMP_USE_HEADER_MAGIC
	uint32_t headerMagic;
#endif
	msgBufRefCount_t refCnt;
	msgPump_MessageBufferFlag_t flags;
}msgPumpMsgHeader_t;

typedef enum en_msgPrioirity
{
	MsgPriorityLow,
	MsgPriorityMedium,
	MsgPriorityHigh
}msgPriority_t;

typedef msgPump_callbackFunction_t msgPumpCallBackVector_t[MAX_AMOUNT_OF_MSG_RECERIVER_PER_MESSAGE];

typedef struct st_msgBufDescription
{
	msgBufBufData_t * const data; 						/* the pointer to the memory region where the message data is stored */
	const msgBufMsgSize_t const msgSize;						/* the size of a message in bytes  */
	const msgBufPos_t const	msgAmount;						/* the amount of messages which can be stored in this buffer */
	const msgPump_MsgID_t const id;							/* the id of this message */
	msgPumpCallBackVector_t * const callbackVector;
	uint32_t padding1;
	uint32_t padding2;
}msgBufDescription_t;

#define MSG_BUFFER_MSG_OVERHEAD (sizeof(msgPumpMsgHeader_t))

#define MSG_PUMP_DECLARE_MESSAGE_BUFFER_POOL(bufferName, messageType, bufferSize, messageID) \
		/* the actual memory to be used */\
		msgPumpCallBackVector_t bufferName ## _callbackVector;\
		/* the memory of the buffer is aligned like: refCount1, flag, messageBuf1, refCount2, flag, messageBuf2.. etc */\
		msgBufBufData_t  bufferName ## _data_p [bufferSize * (sizeof(messageType) + MSG_BUFFER_MSG_OVERHEAD)]; \
			\
		__attribute__ ((unused))\
		__attribute__ ((section(".msgPumpDescriptions")))\
		const msgBufDescription_t bufferName ## _description = \
												{ \
													bufferName ## _data_p,	\
													sizeof(messageType),	\
													bufferSize,	\
													messageID,	\
													&bufferName ## _callbackVector, \
													0,0 \
												};\

/*
 * register on a message. Adds a callback function to a message
 * @param id the ID of the message to register on
 * @param callbackFunction the function to call if the message occurred
 * @param info the argument to be passed to the callback function
 * @return success. There can be a maximum amount of MAX_AMOUNT_OF_MSG_RECERIVER_PER_MESSAGE receivers per message
 */
bool msgPump_registerOnMessage(const msgPump_MsgID_t id, const msgPump_callbackFunction_t callbackFunction);

/*
 * unregister a function from a message
 * @param id the message's ID to unregister from
 * @param callbackFunction the function to unregister
 */
bool msgPump_unregisterFromMessage(const msgPump_MsgID_t id, const msgPump_callbackFunction_t callbackFunction);

/*
 * post a message into the queue. When doing this the reference count is set to the amount of the callback functions listening on this message
 * @param id the message id message belongs to.
 * @param messageToPost a Pointer to a memory region where the message is. This area has to be equal in size to the messageType which was specified when declaring the buffer.
 * @return success. False if the queue is full.
 */
bool msgPump_postMessage(const msgPump_MsgID_t id, void *messageToPost);

/**
 * Add a reference to this message to prevent this buffer to be overwritten when a new message was posted.
 * The MessagePump decrements the retain count after each callback function returned so use this function if you need access to that data later.
 * @return success. Returns false if the message cannot be locked any more because an overflow would happen.
 */
bool msgPump_lockMessage(void *messageToPost);

/*
 * unlock a message to signal that it's memory space can be used for the data producer.
 * The retain count will be decremented after a callback function returned.
 * This function has to be called if the retain count was incremented via msgPump_lockMesasge().
 * A Message is considered to be free if the amount of unlocks is equal to the amount of locks.
 * @return success. If this operation causes the reference count to be less 0 false.
 */
bool msgPump_unlockMessage(void *messageToPost);

/*
 * if there is a free (not-locked) buffer element in a messageBuffer it can be locked with this function
 * @return success. false if there is no unused buffer element
 */
bool msgPump_getFreeBuffer(const msgPump_MsgID_t id, void **o_destPtr);

/*
 * The infinite loop of the runtime. Inside this function all the messages are pumped to their recipients.
 */
void msgPump_pumpMessage(void);


#endif /* MSG_MSGPUMP_H_ */
