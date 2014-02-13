/* Interface for a magnetometer.  */
#ifndef MAGNETOMETER_H_
#define MAGNETOMETER_H_

#include <flawless/stdtypes.h>

/* 3-axes magnetometer output in milli-gauss
 * The coordinate system is defined in file 9dof.h. */
typedef int16_t magnetoMg_t;
typedef struct
{
	magnetoMg_t XOut;
	magnetoMg_t YOut;
	magnetoMg_t ZOut;
} magnetoDataSetMg_t;

#define MILLI_GAUSS_PER_GAUSS 1000

void magneto_triggerMeasurement();
bool magneto_isConnected();


#endif /* MAGNETOMETER_H_ */
