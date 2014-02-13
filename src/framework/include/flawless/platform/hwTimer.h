#ifndef HW_TIMER_H_
#define HW_TIMER_H_

#include <inttypes.h>

typedef uint32_t hw_timerTicks;
typedef uint32_t timer_TimeInterval_us;


/*
 * functions implemented on hw_timer
 */

hw_timerTicks hw_timerGetTicksForInterval_us(timer_TimeInterval_us interval);

hw_timerTicks hw_timerGetTicksElapsed();

void hw_timerSetupTimer(hw_timerTicks);


#endif /* HW_TIMER_H_ */
