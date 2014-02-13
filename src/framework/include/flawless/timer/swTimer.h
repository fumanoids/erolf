/*
 * msgSystem.h
 *
 *  Created on: 14.11.2011
 *      Author: lutz
 */

#ifndef MSGSYSTEM_H_
#define MSGSYSTEM_H_

#include "../stdtypes.h"

typedef void (*timerCallback_t)(void);

typedef uint32_t timerInterval_t;

bool swTimer_registerOnTimer(const timerCallback_t i_callback, timerInterval_t intervalInMS ,bool oneShot);
bool swTimer_registerOnTimerUS(const timerCallback_t i_callback, timerInterval_t intervalInUS ,bool oneShot);

bool swTimer_unRegisterFromTimer(const timerCallback_t i_callback);

void swTimer_trigger();

#endif /* MSGSYSTEM_H_ */
