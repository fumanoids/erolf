#include "mathOptions.h"
#include "quaternion.h"

#include <stdio.h>
#include <math.h>


#define _ACOS(e) acosf(e)
#define _COS(e) cosf(e)
#define _SIN(e) sinf(e)


void quat_assign(quaternion_t* _dest, quaternion_t* _src)
{
	ASSERT(quat_valid(_dest) == QUATERNION_OK);
	ASSERT(quat_valid(_src) == QUATERNION_OK);

	mat_assign(_dest->vector4d->mat_rep, _src->vector4d->mat_rep);
}

void quat_conjugate(quaternion_t* _dest, quaternion_t* _src)
{
	ASSERT(quat_valid(_dest) == QUATERNION_OK);
	ASSERT(quat_valid(_src) == QUATERNION_OK);
	
	*_dest->scalar = *_src->scalar;
	mat_scale(_dest->vector3d->mat_rep, _src->vector3d->mat_rep, -1.0f);
}


quaternionError_t quat_eq( quaternion_t* _q1, quaternion_t* _q2, quaternionData_t _epsilon)
{
	ASSERT(quat_valid(_q1) == QUATERNION_OK);
	ASSERT(quat_valid(_q2) == QUATERNION_OK);

	quaternionError_t retValue = QUATERNION_FAILED;

	matrixError_t error = mat_eq(_q1->vector4d->mat_rep, _q2->vector4d->mat_rep, _epsilon);
	if(error == MATRIX_OK)
		retValue = QUATERNION_OK;

	return retValue;
}
void quat_interpolate(quaternion_t* _dest, quaternion_t* _q1, quaternion_t* _q2,quaternionData_t _alpha, quaternionData_t _epsilon)
{
	ASSERT(quat_valid(_dest) == QUATERNION_OK);
	ASSERT(quat_valid(_q1) == QUATERNION_OK);
	ASSERT(quat_valid(_q2) == QUATERNION_OK);
	ASSERT(_alpha >= 0.0f);
	ASSERT(_alpha <= 1.0f);

	quaternion_t* in1 = _q1;	
	quaternion_t* in2 = _q2;

	quaternionData_t dot,s1,s2,om,sinom;

	dot = vec_mul_dot(in1->vector4d,in2->vector4d);
	

	if( dot < 0)
	{
		dot = -dot;
		mat_scale(_dest->vector4d->mat_rep, _dest->vector4d->mat_rep, -1.0f);
		in2 = _dest;
	}

	if((1.0-dot) > _epsilon)
	{
		om = _ACOS(dot);
		sinom = _SIN(om);
		s1 = _SIN((1.0-_alpha)*om)/sinom;
		s2 = _SIN(_alpha * om)/sinom;
	}
	else
	{
		s1 = 1.0 - _alpha;
		s2 = _alpha;
	}
	*_dest->scalar = s1* *in1->scalar + s2* *in2->scalar;
	_dest->vector3d->data[0] = s1*in1->vector3d->data[0] + s2*in2->vector3d->data[0];
	_dest->vector3d->data[1] = s1*in1->vector3d->data[1] + s2*in2->vector3d->data[1];
	_dest->vector3d->data[2] = s1*in1->vector3d->data[2] + s2*in2->vector3d->data[2];

	ASSERT(quat_valid(_dest) == QUATERNION_OK);
}


void quat_inv(quaternion_t* _dest, quaternion_t* _src)
{
	ASSERT(quat_valid(_dest) == QUATERNION_OK);
	ASSERT(quat_valid(_src) == QUATERNION_OK);

	quaternionData_t scale = 1.0f/vec_length_sqr(_src->vector4d);
	quat_conjugate(_dest,_src);
	quat_scale(_dest, _dest, scale);

	ASSERT(quat_valid(_dest) == QUATERNION_OK);
}


void quat_mul(quaternion_t* _dest, quaternion_t* _q1, quaternion_t* _q2)
{
	ASSERT(quat_valid(_dest) == QUATERNION_OK);
	ASSERT(quat_valid(_q1) == QUATERNION_OK);
	ASSERT(quat_valid(_q2) == QUATERNION_OK);
	
	VEC_INIT(t1,3);
	VEC_INIT(t2,3);
	VEC_INIT(t3,3);

	mat_scale(t1.mat_rep, _q2->vector3d->mat_rep, *_q1->scalar);
	mat_scale(t2.mat_rep, _q1->vector3d->mat_rep, *_q2->scalar);
	vec_mul_cross(&t3,_q1->vector3d, _q2->vector3d);

	*_dest->scalar = *_q1->scalar * *_q2->scalar - vec_mul_dot(_q1->vector3d,_q2->vector3d);
	mat_add(_dest->vector3d->mat_rep, t1.mat_rep, t2.mat_rep);
	mat_add(_dest->vector3d->mat_rep, _dest->vector3d->mat_rep, t3.mat_rep);
	
	ASSERT(quat_valid(_dest) == QUATERNION_OK);
}



void quat_print(quaternion_t* _q)
{
	ASSERT(quat_valid(_q) == QUATERNION_OK);

	printf("[%f, %fi, %fj, %fk]\n",*_q->scalar,_q->vector3d->data[0],_q->vector3d->data[1],_q->vector3d->data[2]);	
}
void quat_rot_vec(vector_t* _dest, quaternion_t* _q, vector_t* _src)
{
	ASSERT(vec_valid(_dest) == VECTOR_OK);
	ASSERT(vec_valid(_src) == VECTOR_OK);
	ASSERT(quat_valid(_q) == QUATERNION_OK);
	ASSERT(_dest != _src);
	ASSERT(_dest->size == 3);
	ASSERT(_src->size == 3);

	MAT_INIT(tmp,3,3);
	quat_to_mat(&tmp,_q);
	mat_mul(_dest->mat_rep, &tmp, _src->mat_rep);

	ASSERT(vec_valid(_dest) == VECTOR_OK);
}
void quat_rotinv_vec(vector_t* _dest, quaternion_t* _q, vector_t* _src)
{
	ASSERT(vec_valid(_dest) == VECTOR_OK);
	ASSERT(vec_valid(_src) == VECTOR_OK);
	ASSERT(quat_valid(_q) == QUATERNION_OK);
	ASSERT(_dest->size == 3);
	ASSERT(_src->size == 3);
	
	QUAT_INIT(inv);
	quat_inv(&inv, _q);
	quat_rot_vec(_dest,&inv,_src);

	ASSERT(vec_valid(_dest) == VECTOR_OK);
}
void quat_scale(quaternion_t* _dest, quaternion_t* _src, quaternionData_t _scale)
{
	ASSERT(quat_valid(_dest) == QUATERNION_OK);
	ASSERT(quat_valid(_src) == QUATERNION_OK);

	mat_scale(_dest->vector4d->mat_rep, _src->vector4d->mat_rep, _scale);

	ASSERT(quat_valid(_dest) == QUATERNION_OK);
}
void quat_set_rot(quaternion_t* _dest, quaternionData_t _angle, vector_t* _axis)
{
	ASSERT(quat_valid(_dest) == QUATERNION_OK);
	ASSERT(vec_valid(_axis) == VECTOR_OK);

	mat_assign(_dest->vector3d->mat_rep,_axis->mat_rep);
	vec_normalize(_dest->vector3d,_dest->vector3d);
	mat_scale(_dest->vector3d->mat_rep, _axis->mat_rep, _SIN(_angle*0.5f));

	*_dest->scalar = _COS(_angle*0.5f);

	ASSERT(quat_valid(_dest) == QUATERNION_OK);
}

void quat_to_mat(matrix_t* _dest, quaternion_t* _src)
{
	ASSERT(mat_valid(_dest) == MATRIX_OK);
	ASSERT(quat_valid(_src) == QUATERNION_OK);
	ASSERT(_dest->cols == 3);
	ASSERT(_dest->rows == 3);

	quaternionData_t norm = vec_length_sqr(_src->vector4d);
	ASSERT(norm > 0);
	norm = 1.0f/sqrt(norm);

	quaternionData_t a,b,c,d;
	a = _src->vector4d->data[0] * norm;
	b = _src->vector4d->data[1] * norm;
	c = _src->vector4d->data[2] * norm;
	d = _src->vector4d->data[3] * norm;

	_dest->data[0] = a*a + b*b - c*c - d*d;
	_dest->data[1] = 2*b*c - 2*a*d;
	_dest->data[2] = 2*b*d + 2*a*c;

	_dest->data[3] = 2*b*c + 2*a*d;
	_dest->data[4] = a*a - b*b + c*c - d*d;
	_dest->data[5] = 2*c*d - 2*a*b;


	_dest->data[6] = 2*b*d - 2*a*c;
	_dest->data[7] = 2*c*d + 2*a*b;
	_dest->data[8] = a*a - b*b - c*c + d*d;

	ASSERT(mat_valid(_dest) == MATRIX_OK);
}

quaternionError_t quat_valid(quaternion_t* _q)
{
	quaternionError_t retValue = QUATERNION_OK;
	if(_q == NULL
		|| _q->vector3d == NULL
		|| _q->vector4d == NULL
		|| _q->scalar == NULL
		|| vec_valid(_q->vector3d) == VECTOR_FAILED
		|| vec_valid(_q->vector4d) == VECTOR_FAILED
		|| _q->vector3d->size != 3
		|| _q->vector4d->size != 4)
	{
		retValue = QUATERNION_FAILED;
	}
	return retValue;
}

