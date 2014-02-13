/*
 * msg_msgPump.c
 *
 *  Created on: 26.09.2011
 *      Author: lutz
 */

#include <flawless/stdtypes.h>
#include <flawless/platform/system.h>
#include <string.h>
#include <flawless/config/msgIDs.h>
#include <flawless/core/msg_msgPump.h>

#define LOG_LEVEL 0
#include <flawless/logging/logging.h>
#include <flawless/init/systemInitializer.h>

#include <flawless/config/msgPump_config.h>

#include "systemMsgIDs.h"

typedef struct st_msgPumpMsgQueueEntry
{
	msgPump_MsgID_t id; /* the ID of the message */
	msgBufBufData_t *buffer; /* the reference to the buffer */
	msgBufDescription_t *bufferDescription; /* a pointer to the structure describing the buffer */
} msgPumpMsgQueueEntry_t;

#if MAX_AMOUNT_OF_MSG_RECERIVER_PER_MESSAGE > 255
#error too many possible recepients of a message!
#endif

#if MSG_PUMP_MSG_QUEUE_MAX_SIZE > 255
#error msg queue is too big!
#endif

/************ globals ****************/

extern msgBufDescription_t _msgPumpBPHandlesBegin;
extern msgBufDescription_t _msgPumpBPHandlesEnd;

/*
 * Here we remember pointers to all the buffer handles which register
 */
msgBufDescription_t *g_messageBuffersBuffer[MSG_LAST_ID + MSG_ID_SYSTEM_MSG_COUNT] = {NULL};
msgBufDescription_t **g_messageBuffers = &(g_messageBuffersBuffer[MSG_ID_SYSTEM_MSG_COUNT]);

static volatile unsigned int g_msgQueueFront = 0U, g_msgQueueTail = 0U,
		g_msgQueueSize = 0U; /* indexes in the ring buffer */
static volatile msgPumpMsgQueueEntry_t g_msgQueue[MSG_PUMP_MSG_QUEUE_MAX_SIZE];


/*************** internal function ********************/
static void setBufferFlag(msgBufBufData_t *buffer, msgPump_MessageBufferFlag_t flag)
{
	msgPumpMsgHeader_t *header = ((msgPumpMsgHeader_t*) buffer) - 1;
	header->flags = flag;
}

/*
 * register on a message. Adds a callback function to a message
 * @param id the ID of the message to register on
 * @param callbackFunction the function to call if the message occurred
 * @param info the argument to be passed to the callback function
 * @return success. There can be a maximum amount of MAX_AMOUNT_OF_MSG_RECERIVER_PER_MESSAGE receivers per message
 */
bool msgPump_registerOnMessage(const msgPump_MsgID_t id,
		const msgPump_callbackFunction_t i_callbackFunction)
{
	system_mutex_lock();
	bool registerSuccess = FALSE;

	if ((id < MSG_LAST_ID) &&
			(id >= MSG_ID_SYSTEM_FIRST_ID) &&
			(NULL != i_callbackFunction))
	{
		msgBufDescription_t *msgBuffer = g_messageBuffers[id];
		if ((NULL != msgBuffer) && (id == msgBuffer->id))
		{
			/* found the corresponding message buffer! So let's add the callback function */
			int i;
			for (i = 0; i < MAX_AMOUNT_OF_MSG_RECERIVER_PER_MESSAGE; ++i)
			{
				if (NULL == (*msgBuffer->callbackVector)[i] || i_callbackFunction == (*msgBuffer->callbackVector)[i])
				{
					(*msgBuffer->callbackVector)[i] = i_callbackFunction;
					registerSuccess = TRUE;
					break;
				}
			}
		}
	}

	system_mutex_unlock();
	return registerSuccess;
}

/*
 * unregister a function from a message
 * @param id the message's ID to unregister from
 * @param callbackFunction the function to unregister
 */
bool msgPump_unregisterFromMessage(const msgPump_MsgID_t id,
		const msgPump_callbackFunction_t callbackFunction)
{
	system_mutex_lock();
	bool unRegisterSuccess = FALSE;


	if ((id < MSG_LAST_ID) &&
		(NULL != callbackFunction))
	{
		msgBufDescription_t *msgBuffer = g_messageBuffers[id];

		if (id == msgBuffer->id)
		{
			int i;
			/* found the corresponding message buffer! So let's remove the callback function... if it is registered */
			for (i = 0; i < MAX_AMOUNT_OF_MSG_RECERIVER_PER_MESSAGE; ++i)
			{
				if (callbackFunction == (*msgBuffer->callbackVector)[i])
				{
					/* found it! */
					(*msgBuffer->callbackVector)[i] = NULL;
					break;
				}
			}
		}
	}

	system_mutex_unlock();
	return unRegisterSuccess;
}

/*
 * post a message into the queue. When doing this the reference count is set to 1
 * @param bufferName the buffer to post the message into
 * @param messageToPost a Pointer to a memory region where the message is. This area has to be equal in size to the messageType which was specified when declaring the buffer.
 * @return success. False if the queue is full.
 */
bool msgPump_postMessage(const msgPump_MsgID_t id, void *messageToPost)
{
	bool postSuccess = FALSE;
	system_mutex_lock();
	/* find the message buffer */
	if (g_msgQueueFront == g_msgQueueTail && 0 != g_msgQueueSize)
	{
		return postSuccess;
	}
	if (id < MSG_LAST_ID)
	{
		msgBufDescription_t *msgBuffer = g_messageBuffers[id];

		if (id == msgBuffer->id)
		{
			/* found it*/
			if (NULL != messageToPost)
			{
				msgBufBufData_t *msgBuffData = msgBuffer->data;
				const msgBufMsgSize_t msgSize = msgBuffer->msgSize;

				/* if the pointer to messageToPost belongs to an already reserved buffer use it */
				const uint8_t *msgBufPos = (const uint8_t*) messageToPost;
				const uint8_t *basePos = (const uint8_t*) msgBuffData;
				const int32_t ptrOffset = msgBufPos - basePos;
				const int32_t bufLen = (msgBuffer->msgAmount
						* (MSG_BUFFER_MSG_OVERHEAD + msgSize));

				if (ptrOffset >= (int32_t)(MSG_BUFFER_MSG_OVERHEAD) && ptrOffset < bufLen)
				{
					/* the caller used an internal buffer! */
					/* but is the pointer directing to a valid element? */
					const uint16_t msgLenWithOverhead = msgSize
							+ MSG_BUFFER_MSG_OVERHEAD;
					if (MSG_BUFFER_MSG_OVERHEAD == ptrOffset % msgLenWithOverhead)
					{
						msgPumpMsgHeader_t *header = ((msgPumpMsgHeader_t*) messageToPost) - 1;
						if ((1 == header->refCnt)
								&& (MsgPump_MsgBufFlagInUse == header->flags)
#ifdef MSG_PUMP_USE_HEADER_MAGIC
								&& (MSG_PUMP_MAGIC == header->headerMagic)
#endif
								)
						{
							/* no one else is using this buffer element! */
							header->flags = MsgPump_MsgBufFlagPostedNotDispatched;
							header->refCnt = 1U;

							/* insert the message into the queue*/
							g_msgQueue[g_msgQueueTail].buffer = messageToPost;
							g_msgQueue[g_msgQueueTail].bufferDescription =
									msgBuffer;
							g_msgQueue[g_msgQueueTail].id = id;
							g_msgQueueTail = (g_msgQueueTail + 1)
									% MSG_PUMP_MSG_QUEUE_MAX_SIZE;
							++g_msgQueueSize;
							postSuccess = TRUE;
						}
						else
						{
							/* WTF?! this buffer is completely lost!!! I'll mark it as invalid */
	//							*flagPtr = MsgPump_MsgBufFlagBufferNotUsable;
							/* ugly fix: magic recovery */
							header->flags = MsgPump_MsgBufFlagNothing;
							header->refCnt = 0U;
#ifdef MSG_PUMP_USE_HEADER_MAGIC
							header->headerMagic = MSG_PUMP_MAGIC;
#endif
							LOG_ERROR_0("Buffer lost!");
						}
					}
					else
					{
						/* BADBADBAD!!! someone is using this message pump wrong! */
					}
				}
				else
				{
					int i;
					/* the caller used an own buffer so we have to copy it's content into a free buffer */
					/* find some free space in the buffer */
					for (i = 0; i < msgBuffer->msgAmount; ++i)
					{
						msgBufBufData_t *curElement = &(msgBuffData[i
								* (msgSize + MSG_BUFFER_MSG_OVERHEAD)]);
						msgPumpMsgHeader_t *curHeader = (msgPumpMsgHeader_t*) curElement;
						msgBufBufData_t *dataPtr = (msgBufBufData_t*) (curHeader + 1);
						if ((0 == curHeader->refCnt)
#ifdef MSG_PUMP_USE_HEADER_MAGIC
								&& (MSG_PUMP_MAGIC == curHeader->headerMagic)
#endif
								&& (MsgPump_MsgBufFlagNothing == curHeader->flags))
						{
							/* found a free buffer! */
							bool lockSuccess = msgPump_lockMessage(dataPtr);
							if (TRUE == lockSuccess)
							{
								curHeader->flags = MsgPump_MsgBufFlagPostedNotDispatched;
								memcpy(dataPtr, messageToPost, msgSize);

								/* insert the message into the queue*/
								g_msgQueue[g_msgQueueTail].buffer = dataPtr;
								g_msgQueue[g_msgQueueTail].bufferDescription =
										msgBuffer;
								g_msgQueue[g_msgQueueTail].id = id;
								g_msgQueueTail = (g_msgQueueTail + 1)
										% MSG_PUMP_MSG_QUEUE_MAX_SIZE;
								++g_msgQueueSize;
								postSuccess = TRUE;
								system_mutex_unlock();
								break;
							} else
							{
								/* cannot lock this buffer (should not happen anyway) */
								continue;
							}
						}
					}
				}
			} else
			{
				/* the message contains a nullpointer. thats easy to handle */
				/* insert the message into the queue*/
				g_msgQueue[g_msgQueueTail].buffer = messageToPost;
				g_msgQueue[g_msgQueueTail].bufferDescription = msgBuffer;
				g_msgQueue[g_msgQueueTail].id = id;
				g_msgQueueTail = (g_msgQueueTail + 1)
						% MSG_PUMP_MSG_QUEUE_MAX_SIZE;
				++g_msgQueueSize;
				postSuccess = TRUE;
			}
		}
	}
	if (FALSE == postSuccess)
	{
	}
	system_mutex_unlock();
	return postSuccess;
}

/**
 * Add a reference to this message to prevent this buffer to be overwritten when a new message was posted.
 * The MessagePump decrements the retain count after each callback function returned so use this function if you need access to that data later.
 * @return success. Returns false if the message cannot be locked any more because an overflow would happen.
 */
 bool msgPump_lockMessage(void *messageToLock)
{
	bool ret = FALSE;
	if (messageToLock != NULL)
	{
		msgPumpMsgHeader_t *header = ((msgPumpMsgHeader_t *) messageToLock) - 1;
		system_mutex_lock();

		if ((255U > header->refCnt)
				&& (MsgPump_MsgBufFlagBufferNotUsable != header->flags)
#ifdef MSG_PUMP_USE_HEADER_MAGIC
				&& (MSG_PUMP_MAGIC == header->headerMagic)
#endif
				)
		{
			/* we can increment the reference count */
			header->refCnt += 1;
			ret = TRUE;
		} else
		{
			LOG_WARNING_0("cannot lock buffer");
			ret = FALSE;
		}
		system_mutex_unlock();
	}
	return ret;
}

/*
 * unlock a message to signal that it's memory space can be used for the data producer.
 * The retain count will be decremented after a callback function returned.
 * This function has to be called if the retain count was incremented via msgPump_lockMesasge().
 * A Message is considered to be free if the amount of unlocks is equal to the amount of locks.
 * @return success. If this operation causes the reference count to be less 0 false.
 */
bool msgPump_unlockMessage(void *messageToUnlock)
{
	bool ret = FALSE;
	if (NULL != messageToUnlock)
	{
		msgPumpMsgHeader_t *header = ((msgPumpMsgHeader_t *) messageToUnlock) - 1;
		if ((0U < header->refCnt)
				&& (MsgPump_MsgBufFlagBufferNotUsable != header->flags)
#ifdef MSG_PUMP_USE_HEADER_MAGIC
				&& (MSG_PUMP_MAGIC == header->headerMagic)
#endif
				)
		{
			/* we can decrement the reference count */
			header->refCnt -= 1;
			ret = TRUE;
		} else
		{
			LOG_WARNING_0("cannot unlock a buffer!");
		}

		system_mutex_unlock();
	}
	return ret;
}

/*
 * if there is a free (not-locked) buffer element in a messageBuffer it can be locked with this function
 * @return success. false if there is no unused buffer element
 */
bool msgPump_getFreeBuffer(const msgPump_MsgID_t id,
		void **o_destPtr)
{
	bool ret = FALSE;
	int i;
	if ((id < MSG_LAST_ID) && (id > MSG_ID_SYSTEM_FIRST_ID))
	{
		msgBufDescription_t *bufferDescriptor = g_messageBuffers[id];
		if ((NULL != bufferDescriptor) && (NULL != o_destPtr))
		{
			system_mutex_lock();
			for (i = 0; i < bufferDescriptor->msgAmount; ++i)
			{
				msgPumpMsgHeader_t *header = (msgPumpMsgHeader_t*) &(bufferDescriptor->data[i * (bufferDescriptor->msgSize + MSG_BUFFER_MSG_OVERHEAD)]);
				msgBufBufData_t *bufData = (msgBufBufData_t*) (header + 1);
				if ((0 == header->refCnt)
						&& (MsgPump_MsgBufFlagNothing == header->flags)
#ifdef MSG_PUMP_USE_HEADER_MAGIC
						&& (MSG_PUMP_MAGIC == header->headerMagic)
#endif
						)
				{
					/* found a free buffer element */
					*o_destPtr = bufData;
					/* lock it */
					bool lockSuccess = msgPump_lockMessage(*o_destPtr);
					if (TRUE == lockSuccess)
					{
						/* we were able to lock this buffer element so get out of the loop */
						/* this check is necessary since an interrupt can catch the buffer element before we do */
						header->flags = MsgPump_MsgBufFlagInUse;
						ret = TRUE;
						break;
					}
				} else
				{
					*o_destPtr = NULL;
				}
			}
			system_mutex_unlock();
		}
	}

	if (FALSE == ret)
	{
		LOG_WARNING_0("Cannot provide free buffer");
		*o_destPtr = NULL;
	}
	return ret;
}

void msgPump_pumpMessage()
{
	while (TRUE)
	{
		if (g_msgQueueSize != 0)
		{
			LOG_VERBOSE_0("pumping");
			volatile msgPumpMsgQueueEntry_t *message =
					&(g_msgQueue[g_msgQueueFront]);
			uint8_t i;

			(void) setBufferFlag(message->buffer, MsgPump_MsgBufFlagInUse);
			for (i = 0; i < MAX_AMOUNT_OF_MSG_RECERIVER_PER_MESSAGE; ++i)
			{
				if ((*message->bufferDescription->callbackVector)[i] != NULL)
				{
					/* call the callback function */
					(void) ((*message->bufferDescription->callbackVector)[i])(
							message->bufferDescription->id,
							message->buffer);
				}
			}
			(void) setBufferFlag(message->buffer, MsgPump_MsgBufFlagNothing);

			msgPump_unlockMessage(message->buffer);
			system_mutex_lock();
			g_msgQueueFront = (g_msgQueueFront + 1)
					% MSG_PUMP_MSG_QUEUE_MAX_SIZE;
			g_msgQueueSize--;
			system_mutex_unlock();
		}
		else
		{
			/*	system_wait_for_interrupt(); */
		}
	}
}


/*
 * initialize the message pump.
 * All messages produced during the initialization process are discarded till now.
 * All messages produced after this init function will be processed
 */
static void msgPump_init(void);
MODULE_INIT_FUNCTION(msgPump, 2, msgPump_init)
static void msgPump_init(void)
{
		msgBufDescription_t *desc = &_msgPumpBPHandlesBegin;
		system_mutex_lock();

		while (desc < &_msgPumpBPHandlesEnd)
		{
			uint8_t i = 0;

			g_messageBuffers[desc->id] = desc;
			memset(desc->data, 0, (desc->msgAmount * (desc->msgSize + MSG_BUFFER_MSG_OVERHEAD)));

#ifdef MSG_PUMP_USE_HEADER_MAGIC
			/* add some magic */
			for (i = 0U; i < desc->msgAmount; ++i)
			{
				msgPumpMsgHeader_t *header = (msgPumpMsgHeader_t *)&(((uint8_t*)desc->data)[i * (desc->msgSize + MSG_BUFFER_MSG_OVERHEAD)]);
				header->headerMagic = MSG_PUMP_MAGIC;
			}
#endif

			desc += 1;
		}
		memset((void*)g_msgQueue, 0, sizeof(g_msgQueue));
		g_msgQueueFront = 0U;
		g_msgQueueSize = 0U;
		g_msgQueueTail = 0U;
		system_mutex_unlock();
}
