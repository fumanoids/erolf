/*
 * rnd.h
 *
 *  Created on: Jun 13, 2013
 *      Author: lutz
 */

#ifndef RND_H_
#define RND_H_


#include <libopencm3/stm32/memorymap.h>
#include <libopencm3/cm3/common.h>

#define RNG_CR MMIO32(RNG_BASE + 0x00)

#define RNG_CR_RNGEN (1 << 2)
#define RNG_CR_IE    (1 << 3)

#define RNG_SR MMIO32(RNG_BASE + 0x04)

#define RNG_SR_DRDY  (1 << 0)
#define RNG_SR_CECS  (1 << 1)
#define RNG_SR_SECS  (1 << 2)

#define RNG_SR_CEIS  (1 << 5)
#define RNG_SR_SEIS  (1 << 6)


#define RNG_DR MMIO32(RNG_BASE + 0x08)



#endif /* RND_H_ */
