/*
 * msg_time.c
 *
 *  Created on: 14.11.2011
 *      Author: lutz, simon
 */

#include <flawless/platform/system.h>
#include <flawless/timer/swTimer.h>
#include <flawless/init/systemInitializer.h>
#include <flawless/platform/hwTimer.h>

#define SW_TIMER_MAX_HANDLES 32U

#define MICROSECONDS_PER_MILLISECOND 1000U

typedef struct st_callbackHandle
{
	timerCallback_t callback;
	hw_timerTicks interval;
	int64_t tickCounter;
	bool oneShot;
} callbackHandle_t;

static volatile callbackHandle_t g_swTimerHandles[SW_TIMER_MAX_HANDLES];
static volatile bool isHandlingClients = false;

bool swTimer_registerOnTimer(const timerCallback_t i_callback, timerInterval_t intervalInMS ,bool oneShot)
{
	return swTimer_registerOnTimerUS(i_callback, intervalInMS * MICROSECONDS_PER_MILLISECOND, oneShot);
}

bool swTimer_registerOnTimerUS(const timerCallback_t i_callback, timerInterval_t intervalUS ,bool oneShot)
{
	bool ret = false;

	hw_timerTicks ticksToSet = hw_timerGetTicksForInterval_us(intervalUS);
	uint8_t i = 0;

	if (NULL != i_callback)
	{
		system_mutex_lock();
		hw_timerTicks startTime = hw_timerGetTicksElapsed();
		/* registration phase */
		uint8_t freeSlot = SW_TIMER_MAX_HANDLES;            // No free slot found
		for (i = 0; i < SW_TIMER_MAX_HANDLES; ++i)
		{
			if (SW_TIMER_MAX_HANDLES == freeSlot &&         // Found free slot
			    NULL == g_swTimerHandles[i].callback)
			{
				freeSlot = i;
			} else if (i_callback == g_swTimerHandles[i].callback) // Found slot with same callback
			{
				g_swTimerHandles[i].callback    = i_callback;
				g_swTimerHandles[i].interval    = ticksToSet;
				g_swTimerHandles[i].tickCounter = ticksToSet + startTime;
				g_swTimerHandles[i].oneShot     = oneShot;
				ret = true;
				break;
			}
		}
		if (false == ret &&                                 // callback wasn't found, but free slot available
		    SW_TIMER_MAX_HANDLES != freeSlot)
		{
			g_swTimerHandles[freeSlot].callback    = i_callback;
			g_swTimerHandles[freeSlot].interval    = ticksToSet;
			g_swTimerHandles[freeSlot].tickCounter = ticksToSet + startTime;
			g_swTimerHandles[freeSlot].oneShot     = oneShot;
			ret = true;
		}

		/* now update all timers according to the time passed */
		int32_t nextInterval = 0x7fffffff;
		if (false == isHandlingClients)
		{
			isHandlingClients = true;

			const hw_timerTicks ticksElapsed = hw_timerGetTicksElapsed();
			for (i = 0U; i < SW_TIMER_MAX_HANDLES; ++i)
			{
				volatile callbackHandle_t *handle = &(g_swTimerHandles[i]);
				if (NULL != handle->callback)
				{
					handle->tickCounter -= ticksElapsed;
					if (0 >= handle->tickCounter) /* timer ran out */
					{
						/* deregister first if its one shot */
						const timerCallback_t callback = handle->callback;
						if (false == handle->oneShot) /* reschedule? */
						{
							handle->tickCounter += handle->interval;
							handle->tickCounter = MAX(handle->tickCounter, 0LL);
							handle->tickCounter = MIN(handle->tickCounter, handle->interval);
						} else
						{
							handle->callback = NULL;
						}
						(void)(callback)();
					}
					if ((NULL != handle->callback)) /* reschedule? */
					{
						nextInterval = MIN(nextInterval, handle->tickCounter);
					}
				}
			}
			isHandlingClients = false;
		}

		/* setup HW timer */
		if (0x7fffffff != nextInterval)
		{
			/* just some minimal time */
			nextInterval = MAX(nextInterval, 4);
			hw_timerSetupTimer(nextInterval);
		}
		system_mutex_unlock();
	}


	return ret;
}

bool swTimer_unRegisterFromTimer(const timerCallback_t i_callback)
{
	bool ret = false;
	uint8_t i = 0;
	system_mutex_lock();
	if (NULL != i_callback)
	{
		for (i = 0U; i < SW_TIMER_MAX_HANDLES; ++i)
		{
			if (i_callback == g_swTimerHandles[i].callback)
			{
				g_swTimerHandles[i].callback = NULL;
				ret = true;
				break;
			}
		}
	}
	system_mutex_unlock();
	return ret;
}

void swTimer_trigger()
{
	uint8_t i;
	int32_t nextInterval = 0x7fffffff;

	system_mutex_lock();
	if (false == isHandlingClients)
	{
		isHandlingClients = true;
		const hw_timerTicks ticksElapsed = hw_timerGetTicksElapsed();
		for (i = 0U; i < SW_TIMER_MAX_HANDLES; ++i)
		{
			volatile callbackHandle_t *handle = &(g_swTimerHandles[i]);
			if (NULL != handle->callback)
			{
				handle->tickCounter -= ticksElapsed;
				if (0 >= handle->tickCounter) /* timer ran out */
				{
					/* deregister first if its one shot */
					const timerCallback_t callback = handle->callback;
					if (false == handle->oneShot) /* reschedule? */
					{
						handle->tickCounter += handle->interval;
						handle->tickCounter = MAX(handle->tickCounter, 0LL);
						handle->tickCounter = MIN(handle->tickCounter, handle->interval);
					} else
					{
						handle->callback = NULL;
					}
					(void)(callback)();
				}
				if ((NULL != handle->callback)) /* reschedule? */
				{
					nextInterval = MIN(nextInterval, handle->tickCounter);
				}
			}
		}
		isHandlingClients = false;
	}

	if (0x7fffffff != nextInterval)
	{
		/* just some minimal time */
		nextInterval = MAX(nextInterval, 40LL);
		hw_timerSetupTimer(nextInterval);
	}
	system_mutex_unlock();
}

static void swTimerInit(void);
MODULE_INIT_FUNCTION(swTimer, 4, swTimerInit)
static void swTimerInit(void)
{
	uint8_t i;
	for (i = 0; i < SW_TIMER_MAX_HANDLES; ++i)
	{
		g_swTimerHandles[i].callback = NULL;
		g_swTimerHandles[i].interval = 0ULL;
		g_swTimerHandles[i].oneShot = false;
		g_swTimerHandles[i].tickCounter = 0LL;
	}
}
