/*
 * string.c
 *
 *  Created on: Jul 17, 2012
 *      Author: lutz
 */

#include <flawless/stdtypes.h>

void * memset(void *dst, int val, size_t len)
{
	uint32_t i = 0U;
	uint8_t *dstPtr = (uint8_t*)dst;
	for (i = 0U; i < len; ++i)
	{
		*dstPtr = val;
		++dstPtr;
	}
	return dst;
}


void * memcpy(void *dst, const void *src, size_t count)
{
	uint32_t i = 0U;
	uint8_t *dstPtr = (uint8_t*)dst;
	const uint8_t *srcPtr = (uint8_t*)src;
	for (i = 0U; i < count; ++i)
	{
		*dstPtr = *srcPtr;
		++dstPtr;
		++srcPtr;
	}
	return dst;
}

int strcmp(const void *a, const void *b)
{
	int ret = 0;
	const uint8_t *aPtr = (const uint8_t*)a;
	const uint8_t *bPtr = (const uint8_t*)b;

	while ((*aPtr != '\0') &&
			(*aPtr != '\0'))
	{
		if (*aPtr != *bPtr)
		{
			ret = -1;
			break;
		}
		++aPtr;
		++bPtr;
	}

	return ret;
}

int memcmp(const void *a, const void *b, size_t count)
{
	int ret = 0;
	const uint8_t *aPtr = (const uint8_t*)a;
	const uint8_t *bPtr = (const uint8_t*)b;

	uint32_t i = 0U;
	for (i = 0U; i < count; ++i)
	{
		if (aPtr[i] != bPtr[i])
		{
			ret = -1;
			break;
		}
	}
	return ret;
}


int itoa(int integer, char *buffer)
{
    int numDigits = 0;
    /* Check if integer is negative and add on additional digit */
    bool isNegative = 0;
    if (integer < 0) {
        integer = -integer;
        isNegative = 1;
        numDigits++;
    }
    /* Count digits to get each digits position in buffer */
    int tmp = integer;
    do {
        numDigits++;
        tmp /= 10;
    } while (tmp);
    /* Put string terminating null */
    buffer[numDigits] = 0;
    /* Put negative sign at beginning if negative*/
    if (isNegative) buffer[0] = '-';
    // Put digits in buffer (reverse ordering)
    int i = numDigits - 1;
    do {
        buffer[i--] = integer%10 + '0';
        integer /= 10;
    } while (integer);
    if (isNegative) return numDigits-1;
    else return numDigits;
}

