/* Interface for an accelerometer.  */

#ifndef ACCELEROMETER_H_
#define ACCELEROMETER_H_

#include <flawless/stdtypes.h>

/* 3-axes accelerometer output in milli g.
 * The coordinate system is defined in file 9dof.h. */
typedef int16_t accelMg_t;
typedef struct
{
	accelMg_t XOut;
	accelMg_t YOut;
	accelMg_t ZOut;
} accelDataSetMg_t;

#define MILLI_G_PER_G 1000

void accel_triggerMeasurement();
bool accel_isConnected();

#endif /* ACCELEROMETER_H_ */
