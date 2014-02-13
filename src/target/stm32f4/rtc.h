/*
 * rtc.h
 *
 *  Created on: Nov 25, 2012
 *      Author: lutz
 */

#ifndef RTC_H_
#define RTC_H_


typedef struct tag_dateTime
{
	uint8_t year; /* only last two decimals eg. 12 for 2012 */
	uint8_t months;
	uint8_t days;
	uint8_t dayOfWeek; /* 1 = monday... 7 = sonday */

	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
}dateTime_t;

/*
 * return the current time
 */
void getTimeOfDay(dateTime_t *io_time);


#endif /* RTC_H_ */
