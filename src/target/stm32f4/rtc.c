/*
 * rtc.c
 *
 *  Created on: Nov 25, 2012
 *      Author: lutz
 */


#include <flawless/init/systemInitializer.h>
#include <flawless/timer/swTimer.h>

#include <libopencm3/stm32/f4/rtc.h>
#include <libopencm3/stm32/f4/rcc.h>

#include "rtc.h"


void getTimeOfDay(dateTime_t *io_time)
{
	io_time->year  = ((RTC_DR >> RTC_DR_YT_SHIFT) & 0xf) * 10;
	io_time->year += ((RTC_DR >> RTC_DR_YU_SHIFT) & 0xf);

	io_time->months  = ((RTC_DR >> RTC_DR_MT_SHIFT) & 0x1) * 10;
	io_time->months += ((RTC_DR >> RTC_DR_MU_SHIFT) & 0xf);

	io_time->days  = ((RTC_DR >> RTC_DR_DT_SHIFT) & 0x3) * 10;
	io_time->days += ((RTC_DR >> RTC_DR_DU_SHIFT) & 0xf);

	io_time->dayOfWeek  = ((RTC_DR >> RTC_DR_WDU_SHIFT) & 0x7);

	io_time->hours  = ((RTC_TR >> RTC_TR_HT_SHIFT) & 0x3) * 10;
	io_time->hours += ((RTC_TR >> RTC_TR_HU_SHIFT) & 0xf);

	io_time->minutes  = ((RTC_TR >> RTC_TR_MNT_SHIFT) & 0x7) * 10;
	io_time->minutes += ((RTC_TR >> RTC_TR_MNU_SHIFT) & 0xf);

	io_time->seconds  = ((RTC_TR >> RTC_TR_ST_SHIFT) & 0x7) * 10;
	io_time->seconds += ((RTC_TR >> RTC_TR_SU_SHIFT) & 0xf);
}

static void rtc_setTimeAndDate(const dateTime_t* i_time)
{
	/* set time */
	uint32_t hrs_hi = ((i_time->hours / 10) & 0x3) << RTC_TR_HT_SHIFT;
	uint32_t hrs_lo = ((i_time->hours % 10) & 0xf) << RTC_TR_HU_SHIFT;

	uint32_t mins_hi = ((i_time->minutes / 10) & 0x7) << RTC_TR_MNT_SHIFT;
	uint32_t mins_lo = ((i_time->minutes % 10) & 0xf) << RTC_TR_MNU_SHIFT;

	uint32_t secs_hi = ((i_time->seconds / 10) & 0x7) << RTC_TR_ST_SHIFT;
	uint32_t secs_lo = ((i_time->seconds % 10) & 0xf) << RTC_TR_SU_SHIFT;

	RTC_TR = hrs_hi | hrs_lo | mins_hi | mins_lo | secs_hi | secs_lo;

	/* set day */
	uint32_t yr_hi = ((i_time->year / 10) & 0xf) << RTC_DR_YT_SHIFT;
	uint32_t yr_lo = ((i_time->year % 10) & 0xf) << RTC_DR_YU_SHIFT;

	uint32_t dow = ((i_time->dayOfWeek) & 0x7) << RTC_DR_WDU_SHIFT;

	uint32_t mt_hi = ((i_time->months / 10) & 0x1) << RTC_DR_MT_SHIFT;
	uint32_t mt_lo = ((i_time->months % 10) & 0xf) << RTC_DR_MU_SHIFT;

	uint32_t day_hi = ((i_time->days / 10) & 0x3) << RTC_DR_DT_SHIFT;
	uint32_t day_lo = ((i_time->days % 10) & 0xf) << RTC_DR_DU_SHIFT;

	RTC_DR = yr_hi | yr_lo | dow | mt_hi | mt_lo | day_hi | day_lo;
}

static void rtc_firstInit(void);
static void rtc_firstInit(void)
{
	RCC_APB1ENR |= RCC_APB1ENR_PWREN;

	PWR_CR |= PWR_CR_DBP;

	/* cause a backup domain reset to select the clock source for RTC */
	RCC_BDCR |= RCC_BDCR_BDRST;
	RCC_BDCR &= ~RCC_BDCR_BDRST;

	/* select HSE as clock source for RTC*/
	RCC_BDCR |= RCC_BDCR_RTCSEL_HSE;

	/* enable RTC */
	RCC_BDCR |= RCC_BDCR_RTCEN;

	/* enable access to RTC */
	RTC_WPR = (0xCA);
	RTC_WPR = (0x53);

	RTC_ISR |= (1 << RTC_ISR_INIT_SHIFT);
	while (0 == (RTC_ISR & (1 << RTC_ISR_INITF_SHIFT)));

	RTC_PRER = (99 << RTC_PRER_PREDIV_A_SHIFT); /* asynch prescaler to 100 */
	RTC_PRER |= (9999 << RTC_PRER_PREDIV_S_SHIFT);


	/* set date and time */
	dateTime_t time;
	time.dayOfWeek = 4;

	time.days = 01;
	time.months = 01;
	time.year =  70;

	time.hours= 00;
	time.minutes = 00;
	time.seconds = 00;

	rtc_setTimeAndDate(&time);


	RTC_ISR &= ~(1 << RTC_ISR_INIT_SHIFT);
}


static void rtc_init(void);
MODULE_INIT_FUNCTION(rtc, 5, rtc_init)
static void rtc_init(void)
{
	if (0 == (RTC_ISR & (1 << RTC_ISR_INITS_SHIFT)))
	{
		rtc_firstInit();
	}
}


