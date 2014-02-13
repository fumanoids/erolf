/*
 * hwTimer.c
 *
 *  Created on: Nov 1, 2012
 *      Author: lutz
 */


#include <flawless/platform/hwTimer.h>
#include <flawless/timer/swTimer.h>

#include <flawless/init/systemInitializer.h>

#include <libopencm3/stm32/f4/timer.h>
#include <libopencm3/stm32/f4/rcc.h>
#include <libopencm3/stm32/nvic.h>

#include "clock.h"

#define HW_TIMER TIM5

#define USECONDS_PER_SECOND 1000000ULL

void tim5_isr()
{
	swTimer_trigger();
	TIM_SR(HW_TIMER) = 0;
}


hw_timerTicks hw_timerGetTicksForInterval_us(timer_TimeInterval_us i_interval)
{
	hw_timerTicks ret = i_interval * (CLOCK_APB1_TIMER_CLK / USECONDS_PER_SECOND);
	return ret;
}

hw_timerTicks hw_timerGetTicksElapsed()
{
	hw_timerTicks ret = (hw_timerTicks)(TIM_PSC(HW_TIMER) + 1) * (hw_timerTicks)(TIM_CNT(HW_TIMER));
	return ret;
}

void hw_timerSetupTimer(hw_timerTicks i_ticks)
{
	const uint32_t psc = ((i_ticks & 0xffffffff00000000) >> 32U);
	const uint32_t arr = (i_ticks / (psc + 1U));

	TIM_CR1(HW_TIMER) &= ~TIM_CR1_CEN; /* disable timer */

	TIM_ARR(HW_TIMER) = 0xffffffff;
	TIM_PSC(HW_TIMER) = psc;
	TIM_CNT(HW_TIMER) = 0U;

	/* disable interrupt */

	/* generate an update event to update the contents of PSC and ARR */
	TIM_SR(HW_TIMER) = 0;
	TIM_EGR(HW_TIMER) = TIM_EGR_UG;
	TIM_SR(HW_TIMER) = 0;

	TIM_CCR1(HW_TIMER) = arr;

	/* re-enable interrupt */
	TIM_DIER(HW_TIMER) |= TIM_DIER_CC1IE;

	/* test if this improves things! */
	TIM_CR1(HW_TIMER) |= TIM_CR1_OPM;

	/* enable timer */
	TIM_CR1(HW_TIMER) |= TIM_CR1_CEN;
}

static void init(void);
MODULE_INIT_FUNCTION(timer, 2, init)
static void init(void)
{
	/* Enable TIM clock. */
	RCC_APB1ENR |= RCC_APB1ENR_TIM5EN;
	/* Enable TIM interrupt. */
	nvic_enable_irq(NVIC_TIM5_IRQ);
	nvic_set_priority(NVIC_TIM5_IRQ, 0xff);

	TIM_PSC(HW_TIMER) = 0U;
	TIM_CNT(HW_TIMER) = 0U;
	TIM_DIER(HW_TIMER) = 0U;
}

