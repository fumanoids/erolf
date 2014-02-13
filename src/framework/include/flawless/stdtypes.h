#ifndef STDTYPES_H
#define STDTYPES_H

#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "misc/Assert.h"

#define UNUSED(x) (void)(x)

#ifndef FALSE
#define FALSE (1 == 0)
#endif

#ifndef TRUE
#define TRUE (1 == 1)
#endif

#ifndef MAX
#define MAX(a,b) ((a < b)? b:a)
#endif

#ifndef MIN
#define MIN(a,b) ((a > b)? b:a)
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

#endif
