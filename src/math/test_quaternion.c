#include "math_test.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "quaternion.h"




void test_quaternion(void)
{
	INIT_TEST


	//**************
	// Test init
	int i;
	for(i = 0; i<10; ++i)
	{
		QUAT_INIT(q);
		TEST(q.scalar != NULL)
		TEST(q.vector3d != NULL);
		TEST(q.vector4d != NULL);
	}
	//**************
	// Test set full
	{
		QUAT_INIT_SET(q,0.0f,1.0f,2.0f,3.0f);
		TEST(*q.scalar == 0.0f
			&& q.vector3d->data[0] == 1.0f
			&& q.vector3d->data[1] == 2.0f
			&& q.vector3d->data[2] == 3.0f
			&& q.vector4d->data[0] == 0.0f
			&& q.vector4d->data[1] == 1.0f
			&& q.vector4d->data[2] == 2.0f
			&& q.vector4d->data[3] == 3.0f
			);
	}
	//**************
	// Test test eq
	{
		QUAT_INIT_SET(q1,0.0f,1.0f,2.0f,3.0f);
		QUAT_INIT_SET(q2,0.0f,1.0f,2.0f,3.0f);
		QUAT_INIT_SET(q3,0.0f,1.0f,2.0f,4.0f);

		TEST(quat_eq(&q1, &q2, 1e-5) == QUATERNION_OK);
		TEST(quat_eq(&q1, &q2, 1e-5) == QUATERNION_OK);
		TEST(!quat_eq(&q1, &q3, 1e-5) == QUATERNION_OK);
		TEST(!quat_eq(&q2, &q3, 1e-5) == QUATERNION_OK);
	}

	//**************
	// Test test conj
	{
		QUAT_INIT_SET(in1,2.0f,1.0f,-2.0f,3.0f);
		QUAT_INIT(res);
		QUAT_INIT_SET(out,2.0f,-1.0f,2.0f,-3.0f);

		quat_conjugate(&res, &in1);

		TEST(quat_eq(&res, &out, 1e-5) == QUATERNION_OK);
	}
	//**************
	// Test test inv
	{
		QUAT_INIT_SET(in1,1.0f,2.0f,-4.0f,2.0f);
		QUAT_INIT(res);
		QUAT_INIT_SET(out,1.0f,-2.0f,4.0f,-2.0f);
		quat_scale(&out,&out,1/25.0f);

		quat_inv(&res, &in1);

		TEST(quat_eq(&res, &out, 1e-5) == QUATERNION_OK);
	}




	//**************
	// Test test mul
	{
		QUAT_INIT_SET(in1,2.0f,1.0f,2.0f,3.0f);
		QUAT_INIT_SET(in2,1.0f,2.0f,3.0f,3.0f);
		QUAT_INIT(res);
		QUAT_INIT_SET(out,-15.0f,2.0f,11.0f,8.0f);

		quat_mul(&res, &in1, &in2);

		TEST(quat_eq(&res, &out, 1e-5) == QUATERNION_OK);
	}
	//**************
	// Test test scale
	{
		QUAT_INIT_SET(in1,2.0f,1.0f,2.0f,-3.0f);
		QUAT_INIT(res);
		QUAT_INIT_SET(out,4.0f,2.0f,4.0f,-6.0f);

		quat_scale(&res, &in1, 2.0f);

		TEST(quat_eq(&res, &out, 1e-5) == QUATERNION_OK);
	}

	
	END_TEST("quaternion")
}
