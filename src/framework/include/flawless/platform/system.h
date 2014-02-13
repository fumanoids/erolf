/*
 * transport.h
 *
 *  Created on: Jun 8, 2012
 *      Author: lutz
 */

#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <flawless/stdtypes.h>

/*
 * here are the most important platform dependant functions listed.
 * The framework cannot work correctly if those functions are not implemented correctly!
 */


/*
 * mutex functions (eg. suspend all interrupts)
 * The mutex functions must be safe for recursive usage!
 */
void system_mutex_lock(void);
void system_mutex_unlock(void);

typedef uint64_t systemTime_t; //In US
/*
 * the current time
 */
systemTime_t getSystemTimeUS();

#endif /* SYSTEM_H_ */
