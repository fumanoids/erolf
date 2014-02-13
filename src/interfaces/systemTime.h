/*
 * systemTime.h
 *
 *  Created on: Apr 6, 2013
 *      Author: lutz
 */

#ifndef SYSTEMTIME_H_
#define SYSTEMTIME_H_
#include <inttypes.h>

typedef uint64_t systemTime_t; //In US
typedef uint64_t syncedTime_t; //In MS

systemTime_t getSystemTimeUS();

/* CurrentTime synchronized with odroid
 * If Time is not synchronized yet, this function will return 0
 */
syncedTime_t getCurrentTime();

/* CurrentTime synchronized with odroid in us
 * If Time is not synchronized yet, this function will return 0
 */
syncedTime_t getCurrentTimeUS();

/* Get Synctime in US of a unsync value from getSystemTimeUS()
 */
syncedTime_t convertToSync(systemTime_t time);

/* Get Synctime in US of a unsync value from getSystemTimeUS()
 */
syncedTime_t convertToSyncUS(systemTime_t time);

// Set timerdifferents, diff says how far the time is behind odroid time
// @param _diff in US
void setCurrentTimeDiffUS(int64_t _diffUS);

/* check if time is still synced
 * 0: is not synced
 * 1: is synced
 */
uint8_t timeIsSync();

/* change state of timer */
void timeSetSync(uint8_t state);

#endif /* SYSTEMTIME_H_ */
