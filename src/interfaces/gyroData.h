/*
 * gyroData.h
 *
 *  Created on: Nov 26, 2012
 *      Author: lutz
 */

#ifndef GYRODATA_H_
#define GYRODATA_H_

typedef struct _imuData_t
{
	uint64_t timestampUS;
	float v[3];
} imuData_t;

#endif /* GYRODATA_H_ */
