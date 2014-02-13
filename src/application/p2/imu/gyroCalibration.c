#include "gyroCalibration.h"

#include "kalmanFilter.h"

#include <interfaces/configuration.h>


/* Functions to compute and apply gyro calibration
 * 
 * To calibrate gyro data, we measure the mean.
 * The mean is applied by substracting the mean of
 * the measured values.
 */

// Calibration mode of gyro and mag (0: off, 1: on)
static uint8_t g_calibrationGyroMode = 0;

// helper variable for calibrating gyro
static uint64_t g_gyroTimeCt = 0;


/* activate gyroscope calibration mode
 *
 */
void startGyroCalibration(float* gyroMean)
{
	if (g_calibrationGyroMode == 0)
	{
		g_gyroTimeCt = 0;

		uint8_t i;
		for (i=0; i<3; ++i)
			gyroMean[i] = 0.0f;

		g_calibrationGyroMode = 1;
	}
}
/* run and/or compute calibration offset for gyroscope calibration
 *
 */
void runGyroCalibration(float* gyroMean, uint64_t timestampUS, float* gyroData)
{
	uint8_t i;
	static uint64_t lastSeenUS = 0;

	if (lastSeenUS == 0) {
		lastSeenUS = timestampUS;
		return;
	}

	uint64_t timediff = timestampUS - lastSeenUS;
	lastSeenUS = timestampUS;

	if (g_calibrationGyroMode == 1)
	{
		g_gyroTimeCt += timediff;

		// calibrationg gyro mean
		for (i = 0; i<3; ++i)
			gyroMean[i] += gyroData[i] * ((float)timediff) * 0.000001f;


		for (i = 0; i<3; ++i)
			gyroData[i] -= gyroMean[i] / (((float)g_gyroTimeCt) * 0.000001f);

	} else {
		for (i = 0; i<3; ++i)
			gyroData[i] -= gyroMean[i];
	}


}
/* deactivate gyro calibration mode
 *
 */
void stopGyroCalibration(float* gyroMean)
{
	if (g_calibrationGyroMode == 1)
	{
		uint8_t i;
		for (i = 0; i<3; ++i)
			gyroMean[i] /= (((float)g_gyroTimeCt) * 0.000001f);

		g_calibrationGyroMode = 0;

		config_updateToFlash();
	}
}


