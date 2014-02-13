#include "math_test.h"

void test_matrix(void);
void test_quaternion(void);
void test_vector(void);

void test_all(void)
{
	test_matrix();
	test_quaternion();
	test_vector();
}

