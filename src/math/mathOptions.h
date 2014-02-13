#ifndef MATH_MATHOPTIONS_H
#define MATH_MATHOPTIONS_H

#ifndef DEBUG_MATH
	#include <flawless/misc/Assert.h>
	#define printf(...)
#else
	#include <assert.h>
	#define ASSERT(e) assert(e)
#endif

#endif
