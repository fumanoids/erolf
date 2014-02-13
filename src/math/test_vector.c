#include "math_test.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "vector.h"




# define M_PI		3.14159265358979323846	/* pi */
# define M_PI_2		1.57079632679489661923	/* pi/2 */
# define M_PI_4		0.78539816339744830962	/* pi/4 */

void test_vector(void)
{
	INIT_TEST


	//**************
	// Test init
	VEC_INIT(v,5);
	TEST(v.size == 5
		&& v.data != 0);

	//**************
	// Test set full
	{
		VEC_INIT_SET(v,2, 0.0f, 1.0f);
		TEST(v.data[0] == 0.0f
			&& v.data[1] == 1.0f);
	}
	//**************
	// Test test eq
	{
		VEC_INIT_SET(v1,4, 0.0f,1.0f,2.0f,3.0f);
		VEC_INIT_SET(v2,4, 0.0f,1.0f,2.0f,3.0f);
		VEC_INIT_SET(v3,4, 0.0f,1.0f,2.0f,4.0f);

		TEST(mat_eq(v1.mat_rep, v2.mat_rep, 1e-5) == MATRIX_OK);
		TEST(mat_eq(v1.mat_rep, v2.mat_rep, 1e-5) == MATRIX_OK);
		TEST(!mat_eq(v1.mat_rep, v3.mat_rep, 1e-5) == MATRIX_OK);
		TEST(!mat_eq(v2.mat_rep, v3.mat_rep, 1e-5) == MATRIX_OK);
	}


	//**************
	// Test mul_cross
	{
		VEC_INIT_SET(v1,3,1.0f,2.0f,3.0f);
		VEC_INIT_SET(v2,3,-7.0f,8.0f,9.0f);
		VEC_INIT_SET(v3,3,-6.0f,-30.0f,22.0f);
		VEC_INIT(v4,3);

		vec_mul_cross(&v4,&v1,&v2);
		TEST(mat_eq(v4.mat_rep, v3.mat_rep, 1e-5) == MATRIX_OK);
	}

	
	END_TEST("vector")
}
