#ifndef NINEDOF_H_
#define NINEDOF_H_

#define USE_9DOF
#define IMU_MEASUREMENT_INTERVALL_MS 25U

/*
 * A 9dof sensor stick provides 9 degree of measurement, by using a 3-axes accelerometer, a 3-axes gyroscop, and a 3-axes magnetometer.
 * You can find the interfaces and especially the data types for those sensors in accelerometer.h, gyro.h and magnetometer.h.
 * This sensor unit is used to measure the orientation of the car in space. The orientation calculation is done by different filters, which uses raw data
 * from one ore more sensors. You can find those in the filter-directory.
 *
 * ### COORDINATE SYSTEM #####
 * The coordinate system  for all sensors and filters is defined as follows:
 * 		The x-axis points towards the driving direction, the y-axis points to the right, the z-axis points down.
 * 		Rotations are defined by three right-handed euler angles:
 * 			1. Rotation about the z-axis by angle psi. (yaw angle or in german "Gierwinkel")
 * 			2. Rotation about the y-axis by angle theta (pitch angle or in german 'Nickwinkel')
 * 			3. Rotation about the x-axis by angle phi (roll angle, or in german "Rollwinkel")
 * */

void switchEndianess16Bit(uint8_t* data);

# define M_PI		3.14159265358979323846	/* pi */

#define SWTICH_ENDIANESS_16BIT(data) switchEndianess16Bit((uint8_t*)data);

/* sensor rotation using right-handed euler angles in deg (from -180 to +180 degree) */
typedef float orientationDegree_t;
typedef struct
{
	orientationDegree_t x; /* roll angle phi */
	orientationDegree_t y; /* pitch angle theta */
	orientationDegree_t z ;/* yaw angle psi */
} orientationData_t;

/* temperature of the sensor unit in degree Celsius*/
typedef float temperatureCelsius_t;

void normalizeOrientationData(orientationDegree_t* data);


#endif /* 9DOF_H_ */
