/*
 * kalmanFilter.h
 *
 *  Created on: 25.02.2013
 *      Author: gene
 */

#ifndef KALMANFILTER_H_
#define KALMANFILTER_H_

void kf_init(void);
void kf_reset(void);
void kf_set_parameter(float Q, float R);
void kf_predict(float const* _gyro, float _timediff);
void kf_predict_temp(void);
void kf_update_accel(float const* _accel);
void kf_update_mag(float const* _mag);
float const* kf_getState(void);

#endif /* KALMANFILTER_H_ */
