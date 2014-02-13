#ifndef IMU_OFFSETCALIBRATION_H
#define IMU_OFFSETCALIBRATION_H

void startOffsetCalibration(float* offsetQuaternion);
void runOffsetCalibration(float* offsetQuaternion);
void stopOffsetCalibration(float* offsetQuaternion);

#endif
