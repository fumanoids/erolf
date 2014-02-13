/*
 * systemTime.c
 *
 *  Created on: Apr 6, 2013
 *      Author: lutz
 */

#include <flawless/init/systemInitializer.h>
#include <flawless/timer/swTimer.h>

#include <target/stm32f4/clock.h>

#include "interfaces/systemTime.h"

#include <libopencm3/stm32/f4/timer.h>
#include <libopencm3/stm32/f4/rcc.h>
#include <libopencm3/stm32/f4/nvic.h>

#define SYSTEM_TIME_TIMER TIM2

#define SYSTEM_TIME_RESOLUTION 1000000ULL

static uint32_t g_systemTimeHighPart;

void tim2_isr()
{
	g_systemTimeHighPart += 1;
	TIM_SR(SYSTEM_TIME_TIMER) = 0;
}

systemTime_t getSystemTimeUS()
{
	uint64_t ret = TIM_CNT(SYSTEM_TIME_TIMER);
	ret += (((uint64_t)g_systemTimeHighPart) << 32);
	return ret;
}


static void systemTime_init(void);
MODULE_INIT_FUNCTION(systemTime, 3, systemTime_init)
static void systemTime_init(void)
{
	/* Enable TIM clock. */
	RCC_APB1ENR |= RCC_APB1ENR_TIM2EN;

	/* Enable TIM interrupt. */
	nvic_enable_irq(NVIC_TIM2_IRQ);
	nvic_set_priority(NVIC_TIM2_IRQ, 4);

	g_systemTimeHighPart = 0U;
	TIM_PSC(SYSTEM_TIME_TIMER) = CLOCK_APB1_TIMER_CLK / SYSTEM_TIME_RESOLUTION;
	TIM_CNT(SYSTEM_TIME_TIMER) = 0U;
	TIM_ARR(SYSTEM_TIME_TIMER) = 0xffffffff;
	TIM_EGR(SYSTEM_TIME_TIMER) = TIM_EGR_UG;

	TIM_SR(SYSTEM_TIME_TIMER) = 0;
	TIM_DIER(SYSTEM_TIME_TIMER) = TIM_DIER_UIE;

	/* enable timer */
	TIM_CR1(SYSTEM_TIME_TIMER) |= TIM_CR1_CEN;
}




static uint8_t g_timeIsSync = 0;
static uint8_t g_timeDiffInit = 0;
static int64_t g_timeDiffUS = 0;

// CurrentTime synchronized with odroid
syncedTime_t getCurrentTime() {
	return convertToSync(getSystemTimeUS());
}
syncedTime_t getCurrentTimeUS() {
	return convertToSyncUS(getSystemTimeUS());
}

syncedTime_t convertToSyncUS(systemTime_t time)
{
	if (g_timeDiffInit == 1) {
		time = (time + g_timeDiffUS);
	} else {
		time = 0ULL;
	}
	return time;
}
syncedTime_t convertToSync(systemTime_t time)
{
	return convertToSyncUS(time) / 1000ULL;
}

// Set timerdifferents, diff says how far the time is behind odroid time
void setCurrentTimeDiffUS(int64_t _diffUS) {
	g_timeDiffUS = _diffUS;
	g_timeDiffInit = 1;
}

uint8_t timeIsSync() {
	return g_timeIsSync;
}
void timeSetSync(uint8_t state) {
	g_timeIsSync = state;
}
