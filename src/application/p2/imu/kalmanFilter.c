#include "kalmanFilter.h"

#include <math/quaternion.h>
#include <math.h>

STATIC_QUAT_INIT_SET(state,1,0,0,0)
STATIC_MAT_INIT(P,4,4)
STATIC_MAT_INIT(Raccel,3,3)
STATIC_MAT_INIT(Rmag,3,3)
STATIC_MAT_INIT(Q,4,4)


void kf_init(void)
{
	state.vector4d->data[0] = 1.0f;
	state.vector4d->data[1] = 0.0f;
	state.vector4d->data[2] = 0.0f;
	state.vector4d->data[3] = 0.0f;
	mat_eye(&P);
	mat_scale(&P, &P, 1);

	mat_eye(&Q);
	mat_scale(&Q, &Q, 0.0005);

	mat_eye(&Raccel);
	mat_scale(&Raccel, &Raccel, 5.0);

	mat_eye(&Rmag);
	mat_scale(&Rmag, &Rmag, 0.01);
}
void kf_reset(void)
{
	mat_eye(&P);
}
void kf_set_parameter(float _Q, float _R)
{
	mat_eye(&Q);
	mat_scale(&Q, &Q, _Q);

	mat_eye(&Raccel);
	mat_scale(&Raccel, &Raccel, _R);
}


void kf_predict(float const* _gyro, float _timediff)
{
	VEC_INIT(_angle, 3);
	_angle.data[0] = _gyro[0];
	_angle.data[1] = _gyro[1];
	_angle.data[2] = _gyro[2];

	mat_scale(_angle.mat_rep, _angle.mat_rep, _timediff);

	/* Prediction:
	 * 1. Predict state: x_(k+1) = f(x_k) = x_k * w
	 * 2. Predict covariance: P_k = A_k*P_(k-1)*A_k' + Q_k
	 */
	// 1. Predict state
	float cv0 = cosf(_angle.data[0]*0.5);
	float cv1 = cosf(_angle.data[1]*0.5);
	float cv2 = cosf(_angle.data[2]*0.5);
	float sv0 = sinf(_angle.data[0]*0.5);
	float sv1 = sinf(_angle.data[1]*0.5);
	float sv2 = sinf(_angle.data[2]*0.5);

	QUAT_INIT(w);
	w.vector4d->data[0] = cv0*cv1*cv2 - sv0*sv1*sv2;
	w.vector4d->data[1] = sv0*cv1*cv2 + cv0*sv1*sv2;
	w.vector4d->data[2] = cv0*sv1*cv2 - sv0*cv1*sv2;
	w.vector4d->data[3] = cv0*cv1*sv2 + sv0*sv1*cv2;




	// 2. Predict covariance
	MAT_INIT(F, 4, 4);
	MAT_INIT(Ft, 4, 4);
	F.data[ 0 + 0] = w.vector4d->data[0];
	F.data[ 4 + 0] = w.vector4d->data[1];
	F.data[ 8 + 0] = w.vector4d->data[2];
	F.data[12 + 0] = w.vector4d->data[3];

	F.data[ 0 + 1] = -w.vector4d->data[1];
	F.data[ 4 + 1] =  w.vector4d->data[0];
	F.data[ 8 + 1] =  w.vector4d->data[3];
	F.data[12 + 1] = -w.vector4d->data[2];

	F.data[ 0 + 2] = -w.vector4d->data[2];
	F.data[ 4 + 2] = -w.vector4d->data[3];
	F.data[ 8 + 2] =  w.vector4d->data[0];
	F.data[12 + 2] =  w.vector4d->data[1];

	F.data[ 0 + 3] = -w.vector4d->data[3];
	F.data[ 4 + 3] =  w.vector4d->data[2];
	F.data[ 8 + 3] = -w.vector4d->data[1];
	F.data[12 + 3] =  w.vector4d->data[0];
	mat_transpose(&Ft, &F);


	//x = F * x;
	MAT_INIT(t_state, 4, 1);
	mat_assign(&t_state, state.vector4d->mat_rep);
	mat_mul(state.vector4d->mat_rep, &F, &t_state);


	//P = F*P*F'+Q
	MAT_INIT(t1, 4, 4);
	MAT_INIT(t2, 4, 4);
	MAT_INIT(t3, 4, 4);
	mat_mul(&t1, &F, &P);
	mat_mul(&t2, &t1, &Ft);
	mat_scale(&t3, &Q, _timediff);
	mat_add(&P, &t2, &t3);
}
void kf_predict_temp(void)
{
	mat_add(&P, &P, &Q);
}

void kf_update_accel(float const* _accel)
{
	VEC_INIT(accel, 3);
	accel.data[0] = _accel[0];
	accel.data[1] = _accel[1];
	accel.data[2] = _accel[2];
	vec_normalize(&accel, &accel);

	/* Update Measurement:
	 * 0. H/Ht
	 * 1. S = (H*P*Ht + R)'
	 * 2. kalman gain: K_k = P*Ht * S
	 * 3. state estimate: x = x + K (z - H*x)
	 * 4. covariance update: P = (I - K*H)*P
	 */
	/* Update Measurement:
	 * 0. H
	 * 1. y = z - hx
	 * 2. S = H*P*H'+R
	 * 3. K = P*H'*S^-1
	 * 4. x = x+K*y
	 * 5. P = (I - K*H)*P
	 */
		// 0. H/Ht
	MAT_INIT(H, 3, 4);
	MAT_INIT(Ht, 4, 3);
	{
		mat_set(&H,0,0, 2.0f*state.vector4d->data[2]);
		mat_set(&H,1,0,-2.0f*state.vector4d->data[1]);
		mat_set(&H,2,0, 2.0f*state.vector4d->data[0]);

		mat_set(&H,0,1, 2.0f*state.vector4d->data[3]);
		mat_set(&H,1,1,-2.0f*state.vector4d->data[0]);
		mat_set(&H,2,1,-2.0f*state.vector4d->data[1]);

		mat_set(&H,0,2, 2.0f*state.vector4d->data[0]);
		mat_set(&H,1,2, 2.0f*state.vector4d->data[3]);
		mat_set(&H,2,2,-2.0f*state.vector4d->data[2]);

		mat_set(&H,0,3, 2.0f*state.vector4d->data[1]);
		mat_set(&H,1,3, 2.0f*state.vector4d->data[2]);
		mat_set(&H,2,3, 2.0f*state.vector4d->data[3]);
		mat_transpose(&Ht, &H);
	}
	// 1. y = z - Hx
	VEC_INIT(y, 3);
	{
		VEC_INIT(Hx, 3);
		mat_mul(Hx.mat_rep, &H, state.vector4d->mat_rep);
		mat_sub(y.mat_rep, accel.mat_rep, Hx.mat_rep);
	}
	// 2. S = H*P*H'+R
	MAT_INIT(S, 3,3);
	{
		MAT_INIT(HP, 3, 4);
		mat_mul(&HP, &H, &P);
		MAT_INIT(HPHt, 3, 3);
		mat_mul(&HPHt, &HP, &Ht);

		mat_add(&S, &HPHt, &Raccel);
	}
	// 3. K = P*Ht*S^-1
	MAT_INIT(K, 4, 3);
	{
		MAT_INIT(PHt, 4, 3);
		mat_mul(&PHt, &P, &Ht);
		MAT_INIT(Sinv, 3, 3);
		MAT_INIT(St1, 3, 3);
		MAT_INIT(St2, 2, 2);
		mat_inv(&Sinv, &S, &St1, &St2);
		mat_mul(&K, &PHt, &Sinv);
		}
	// 4. x = x+K*y
	{
		VEC_INIT(Ky, 4);
		mat_mul(Ky.mat_rep, &K, y.mat_rep);
		mat_add(state.vector4d->mat_rep, state.vector4d->mat_rep, Ky.mat_rep);
	}
	// 5. P = (I - K*H)*P
	{
		MAT_INIT(I, 4, 4);
		mat_eye(&I);
		MAT_INIT(KH, 4, 4);
		mat_mul(&KH, &K, &H);
		MAT_INIT(IKH, 4, 4);
		mat_sub(&IKH, &I, &KH);

		MAT_INIT(Pt, 4, 4);
		mat_assign(&Pt, &P);
		mat_mul(&P, &IKH, &Pt);
		}
}
void kf_update_mag(float const* _mag)
{
	VEC_INIT(mag, 3);
	mag.data[0] = _mag[0];
	mag.data[1] = _mag[1];
	mag.data[2] = _mag[2];
	vec_normalize(&mag, &mag);

	/* Update Measurement:
	 * 0. H/Ht
	 * 1. S = (H*P*Ht + R)'
	 * 2. kalman gain: K_k = P*Ht * S
	 * 3. state estimate: x = x + K (z - H*x)
	 * 4. covariance update: P = (I - K*H)*P
	 */
	/* Update Measurement:
	 * 0. H
	 * 1. y = z - hx
	 * 2. S = H*P*H'+R
	 * 3. K = P*H'*S^-1
	 * 4. x = x+K*y
	 * 5. P = (I - K*H)*P
	 */
		// 0. H/Ht
	MAT_INIT(H, 3, 4);
	MAT_INIT(Ht, 4, 3);
	{
		mat_set(&H,0,0, -2.0f*state.vector4d->data[0]);
		mat_set(&H,1,0, -2.0f*state.vector4d->data[3]);
		mat_set(&H,2,0,  2.0f*state.vector4d->data[2]);

		mat_set(&H,0,1, -2.0f*state.vector4d->data[1]);
		mat_set(&H,1,1, -2.0f*state.vector4d->data[2]);
		mat_set(&H,2,1, -2.0f*state.vector4d->data[3]);

		mat_set(&H,0,2,  2.0f*state.vector4d->data[2]);
		mat_set(&H,1,2, -2.0f*state.vector4d->data[1]);
		mat_set(&H,2,2,  2.0f*state.vector4d->data[0]);

		mat_set(&H,0,3,  2.0f*state.vector4d->data[3]);
		mat_set(&H,1,3, -2.0f*state.vector4d->data[0]);
		mat_set(&H,2,3, -2.0f*state.vector4d->data[1]);


		mat_transpose(&Ht, &H);
	}
	// 1. y = z - Hx
	VEC_INIT(y, 3);
	{
		VEC_INIT(Hx, 3);
		mat_mul(Hx.mat_rep, &H, state.vector4d->mat_rep);
		mat_sub(y.mat_rep, mag.mat_rep, Hx.mat_rep);
	}
	// 2. S = H*P*H'+R
	MAT_INIT(S, 3,3);
	{
		MAT_INIT(HP, 3, 4);
		mat_mul(&HP, &H, &P);
		MAT_INIT(HPHt, 3, 3);
		mat_mul(&HPHt, &HP, &Ht);

		mat_add(&S, &HPHt, &Rmag);
	}
	// 3. K = P*Ht*S^-1
	MAT_INIT(K, 4, 3);
	{
		MAT_INIT(PHt, 4, 3);
		mat_mul(&PHt, &P, &Ht);
		MAT_INIT(Sinv, 3, 3);
		MAT_INIT(St1, 3, 3);
		MAT_INIT(St2, 2, 2);
		mat_inv(&Sinv, &S, &St1, &St2);
		mat_mul(&K, &PHt, &Sinv);
		}
	// 4. x = x+K*y
	{
		VEC_INIT(Ky, 4);
		mat_mul(Ky.mat_rep, &K, y.mat_rep);
		mat_add(state.vector4d->mat_rep, state.vector4d->mat_rep, Ky.mat_rep);
	}
	// 5. P = (I - K*H)*P
	{
		MAT_INIT(I, 4, 4);
		mat_eye(&I);
		MAT_INIT(KH, 4, 4);
		mat_mul(&KH, &K, &H);
		MAT_INIT(IKH, 4, 4);
		mat_sub(&IKH, &I, &KH);

		MAT_INIT(Pt, 4, 4);
		mat_assign(&Pt, &P);
		mat_mul(&P, &IKH, &Pt);
		}
}

float const* kf_getState(void)
{
	vec_normalize(state.vector4d, state.vector4d);
	return state.vector4d  ->data;
}
