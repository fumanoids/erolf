#include "math_test.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"




# define M_PI		3.14159265358979323846	/* pi */
# define M_PI_2		1.57079632679489661923	/* pi/2 */
# define M_PI_4		0.78539816339744830962	/* pi/4 */


void test_matrix(void)
{
	INIT_TEST


	//**************
	// Test init
	// 1. Test
	{
	MAT_INIT(m,5,5);
	TEST(m.rows == 5
		&& m.cols == 5
		&& m.data != 0);
	}
	// 2. Test
	{
	MAT_INIT(m,4,5);
	TEST(m.rows == 4
		&& m.cols == 5
		&& m.data != 0);
	}
	// 3. Test
	{
	MAT_INIT(m,5,6);
	TEST(m.rows == 5
		&& m.cols == 6
		&& m.data != 0);
	}


	//**************
	// Test set full
	{
		MAT_INIT_SET(m,2,2,  0.0f,1.0f,2.0f,3.0f);
		TEST(m.data[0] == 0.0f
			&& m.data[1] == 1.0f
			&& m.data[2] == 2.0f
			&& m.data[3] == 3.0f);
	}
	//**************
	// Test test eq
	{
		MAT_INIT_SET(m1,2,2, 0.0f,1.0f,2.0f,3.0f);
		MAT_INIT_SET(m2,2,2, 0.0f,1.0f,2.0f,3.0f);
		MAT_INIT_SET(m3,2,2, 0.0f,1.0f,2.0f,4.0f);

		TEST(mat_eq(&m1, &m2,1e-5) == MATRIX_OK);
		TEST(mat_eq(&m1, &m2,1e-5) == MATRIX_OK);
		TEST(mat_eq(&m1, &m3,1e-5) == MATRIX_FAILED);
		TEST(mat_eq(&m2, &m3,1e-5) == MATRIX_FAILED);
	}
	//**************
	// Test test add
	{
		MAT_INIT_SET(m1,2,2,0.0f,1.0f,2.0f,3.0f);
		MAT_INIT_SET(m2,2,2,1.0f,3.0f,5.0f,-3.0f);
		MAT_INIT_SET(m3,2,2,1.0f,4.0f,7.0f,0.0f);
		mat_add(&m1, &m1, &m2);
		TEST(mat_eq(&m1, &m3,1e-5) == MATRIX_OK);
	}
	// Test test adjunct
	{
		{ // Test2
			MAT_INIT_SET(in,2,2,	 1.0f,  2.0f,
									-0.5f, 3.0f);
			MAT_INIT_SET(out,2,2,	3.0f, 0.5f,
									-2.0f,  1.0f);
			MAT_INIT(res,2,2);
			MAT_INIT(tmp,1,1);
			
			mat_adjugate(&res, &in, &tmp);

			TEST(mat_eq(&out, &res,1e-5) == MATRIX_OK);
		}

	}

	//**************
	// Test test assign
	{
		MAT_INIT_SET(m1,2,2, 0.0f,1.0f,2.0f,3.0f);
		MAT_INIT_SET(m2,2,2, 0.0f,1.0f,2.0f,4.0f);


		TEST(mat_eq(&m1, &m2,1e-5) == MATRIX_FAILED);
		mat_assign(&m1, &m2);
		TEST(mat_eq(&m1, &m2,1e-5) == MATRIX_OK);

	}

	//**************
	// Test test det
	{
		{ // eye() always 1.0 and zero always 0.0

		MAT_INIT(m,10,10);
		MAT_INIT(tmp,10,10);
		MAT_INIT(inplace,10,10);
		mat_eye(&m);
		TEST(mat_det(&m, &tmp) == 1.0f);

		mat_zero(&m);
		TEST(mat_det(&m, &tmp) == 0.0f);

		mat_eye(&inplace);
		TEST(mat_det(&inplace, &inplace) == 1.0f);

		mat_zero(&inplace);
		TEST(mat_det(&inplace, &inplace) == 0.0f);


		}

	}


	//**************
	// Test test eye
	{
		MAT_INIT(m1,2,2);
		mat_eye(&m1);

		MAT_INIT_SET(m2,2,2,	1.0f, 0.0f,
								0.0f, 1.0f);
		TEST(mat_eq(&m1, &m2, 1e-5) == MATRIX_OK);
	}
	//**************
	// Test test inv
	{
		// 1. Test
		{ 
			MAT_INIT(eye,15,15);
			mat_eye(&eye);

			MAT_INIT(m,15,15);
			mat_rand(&m);

			MAT_INIT(tmp1,15,15);
			MAT_INIT(tmp2,14,14);

			MAT_INIT(m_i,15,15);
			mat_inv(&m_i, &m, &tmp1, &tmp2);

			MAT_INIT(m_res,15,15);
			mat_mul(&m_res, &m, &m_i);
			TEST(mat_eq(&eye, &m_res, 1e-2) == MATRIX_OK);
		}
		// 2. Test
		int i;
		for(i = 0; i < 10; ++i)
		{ 
			MAT_INIT(eye,3,3);
			mat_eye(&eye);

			MAT_INIT(m,3,3);
			mat_rand(&m);

			MAT_INIT(tmp1,3,3);
			MAT_INIT(tmp2,2,2);

			MAT_INIT(m_i,3,3);
			mat_inv(&m_i, &m, &tmp1, &tmp2);

			MAT_INIT(m_res,3,3);
			mat_mul(&m_res, &m, &m_i);
			TEST(mat_eq(&eye, &m_res, 1e-3) == MATRIX_OK);
		}

	}

	//**************
	// Test test minor
	{
		{ // Test1
			MAT_INIT_SET(m1,3,3,	15.0f, 6.0f,  9.0f,
									 3.0f,  1.0f,  2.0f,
									15.0f, 10.0f,  5.0f);


			MAT_INIT_SET(m2,2,2,	15.0f,  9.0f,
									15.0f,  5.0f);
			MAT_INIT(m3,2,2);
			mat_minor(&m3, &m1,1,1);
			TEST(mat_eq(&m2, &m3, 1e-5) == MATRIX_OK);
		}
		{ // Test2
			MAT_INIT_SET(m1,3,3,	15.0f, 6.0f,  9.0f,
									 3.0f,  1.0f,  2.0f,
									15.0f, 10.0f,  5.0f);


			MAT_INIT_SET(m2,2,2,	15.0f,  9.0f,
									3.0f,  2.0f);
			MAT_INIT(m3,2,2);
			mat_minor(&m3, &m1, 2, 1);
			TEST(mat_eq(&m2, &m3, 1e-5) == MATRIX_OK);
		}

	}

	//**************
	// Test test mul
	{
		{ //Test 1

			MAT_INIT_SET(m1,2,2,	3.0f, 2.0f,
									0.0f, 1.0f);
			MAT_INIT_SET(m2,2,2,	5.0f, -4.0f,
									1.0f, -1.0f);
			MAT_INIT_SET(m3,2,2,	17.0f, -14.0f,
									1.0f, -1.0f);
			MAT_INIT(m4,2,2);
			mat_mul(&m4,&m1,&m2);
			TEST(mat_eq(&m4,&m3,1e-5) == MATRIX_OK);
		}
		{ //Test 2

			MAT_INIT_SET(m1,2,3,	3.0f, 2.0f, 1.0f,
									0.0f, 1.0f, -1.0f);
			MAT_INIT_SET(m2,3,2,	5.0f, -4.0f,
									1.0f, -1.0f,
									5.0f, 0.0f);
			MAT_INIT_SET(m3,2,2,	22.0f, -14.0f,
									-4.0f, -1.0f);
			MAT_INIT(m4,2,2);
			mat_mul(&m4,&m1,&m2);
			TEST(mat_eq(&m4,&m3,1e-5) == MATRIX_OK);
		}
		{ //Test 3

			MAT_INIT_SET(m1,3,2,	5.0f, -4.0f,
									1.0f, -1.0f,
									5.0f, 0.0f);
			MAT_INIT_SET(m2,2,3,	3.0f, 2.0f, 1.0f,
									0.0f, 1.0f, -1.0f);
			MAT_INIT_SET(m3,3,3,	15.0f, 6.0f,  9.0f,
									 3.0f,  1.0f,  2.0f,
									15.0f, 10.0f,  5.0f);
			MAT_INIT(m4,3,3);
			mat_mul(&m4,&m1,&m2);
			TEST(mat_eq(&m4,&m3,1e-5) == MATRIX_OK);
		}
	}
	//**************
	// Test test scale
	{
		
		{ //Test 1
			MAT_INIT_SET(m0,2,2,	3.0f, 2.0f,
									0.0f, -1.0f);
			MAT_INIT_SET(m1,2,2,	6.0f, 4.0f,
									0.0f, -2.0f);
			MAT_INIT(m2,2,2);
			mat_scale(&m2,&m0,2);
			TEST(mat_eq(&m2,&m1,1e-5) == MATRIX_OK);
		}
		{ //Test 2
			MAT_INIT_SET(m0,2,2,	3.0f, 2.0f,
									0.0f, -1.0f);
			MAT_INIT_SET(m1,2,2,	6.0f, 4.0f,
									0.0f, -2.0f);
			mat_scale(&m0,&m0,2);
			TEST(mat_eq(&m0,&m1,1e-5) == MATRIX_OK);
		}
	}
	//**************
	// Test test transpose
	{
		{ //Test 1
			MAT_INIT_SET(m0,2,2,	3.0f, 2.0f,
									0.0f, -1.0f);
			MAT_INIT_SET(m1,2,2,	3.0f, 0.0f,
									2.0f, -1.0f);
			MAT_INIT(m2,2,2);
			mat_transpose(&m2,&m0);
			TEST(mat_eq(&m2,&m1,1e-5) == MATRIX_OK);
		}
		{ //Test 2
			MAT_INIT_SET(m0,3,2,	3.0f, 2.0f,
									0.0f, -1.0f,
									1.0f, -4.0f);
			MAT_INIT_SET(m1,2,3,	3.0f, 0.0f, 1.0f,
									2.0f, -1.0f, -4.0f);
			MAT_INIT(m2,2,3);
			mat_transpose(&m2,&m0);
			TEST(mat_eq(&m2,&m1,1e-5) == MATRIX_OK);
		}
	}
	
	END_TEST("matrix")
}
