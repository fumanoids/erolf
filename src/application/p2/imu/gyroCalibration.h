#ifndef IMU_GYROCALIBRATION_H
#define IMU_GYROCALIBRATION_H

#include "math/quaternion.h"

void startGyroCalibration(float* gyroMean);
void runGyroCalibration(float* gyroMean, uint64_t timestampUS, float* gyroData);
void stopGyroCalibration(float* gyroMean);

#endif

