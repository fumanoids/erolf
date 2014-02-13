/*
 * MoBoAssert.h
 *
 *  Created on: 22.11.2011
 *      Author: lutz
 */

#ifndef MOBOASSERT_H_
#define MOBOASSERT_H_

#include <flawless/stdtypes.h>

#ifdef DEBUG
#undef DEBUG
#endif

#ifdef DEBUG
#define ASSERT(expr) if(false == (expr)) assert_failFunction(__FILE__, #expr, __LINE__);

void assert_failFunction(const char* fileName, const char* expr, const uint16_t lineNo);

#else
/* empty implementation */
#define ASSERT(expr) (void)(expr)
#endif


#endif /* MOBOASSERT_H_ */
