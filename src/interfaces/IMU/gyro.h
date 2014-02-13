/* Interface for a gyroscope. */

#ifndef GYRO_H_
#define GYRO_H_

#include <flawless/stdtypes.h>

/* 3-axes gyro output in milli-degree per second.
 * The coordinate system is defined in file 9dof.h. */
typedef int32_t gyroMdps_t;
typedef struct
{
	gyroMdps_t XOut;
	gyroMdps_t YOut;
	gyroMdps_t ZOut;
} gyroDataSetMdps_t;

#define MILLI_DEGREE_PER_DEGREE 1000

void gyro_triggerMeasurement();
bool gyro_isConnected();

#endif /* GYRO_H_ */
