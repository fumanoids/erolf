#include "mathOptions.h"
#include "vector.h"


#include <stdio.h>
#include <math.h>



vectorData_t vec_length_sqr(vector_t* _v)
{
	ASSERT(vec_valid(_v) == VECTOR_OK);

	vectorData_t retValue = 0.0f;

	vectorScale_t i;
	for(i = 0; i < _v->size; ++i)
	{
		retValue += _v->data[i]*_v->data[i];
	}

	ASSERT(vec_valid(_v) == VECTOR_OK);
	return retValue;
}
void vec_mul_cross(vector_t* _dest, vector_t* _v1, vector_t* _v2)
{
	ASSERT(vec_valid(_dest) == VECTOR_OK);
	ASSERT(vec_valid(_v1) == VECTOR_OK);
	ASSERT(vec_valid(_v2) == VECTOR_OK);
	ASSERT(_v1->size == _v2->size);
	ASSERT(_v1->size == _dest->size);
	ASSERT(_dest != _v1);
	ASSERT(_dest != _v2);

	vectorScale_t n = _v1->size;

	if(n == 1)
	{
		_dest->data[0] = _v1->data[1] * _v2->data[2];
	}
	else if(n == 3)
	{
		_dest->data[0] = _v1->data[1] * _v2->data[2] - _v1->data[2] * _v2->data[1];
		_dest->data[1] = _v1->data[2] * _v2->data[0] - _v1->data[0] * _v2->data[2];
		_dest->data[2] = _v1->data[0] * _v2->data[1] - _v1->data[1] * _v2->data[0];

	}
	else 
	{
		ASSERT(0);//nicht implementieren
	}

	ASSERT(vec_valid(_dest) == VECTOR_OK);
}
vectorData_t vec_mul_dot(vector_t* _v1, vector_t* _v2)
{
	ASSERT(vec_valid(_v1) == VECTOR_OK);
	ASSERT(vec_valid(_v2) == VECTOR_OK);

	return    _v1->data[0] * _v2->data[0]
			+ _v1->data[1] * _v2->data[1]
			+ _v1->data[2] * _v2->data[2];

}
void vec_normalize(vector_t* _dest, vector_t* _src)
{
	ASSERT(vec_valid(_dest) == VECTOR_OK);
	ASSERT(vec_valid(_src) == VECTOR_OK);
	ASSERT(_dest->size == _src->size);

	vectorData_t length = sqrtf(vec_length_sqr(_src));
	if(length > 0.0)
	{
		vectorData_t scale = 1.0f/length;
		vectorScale_t i;
		for(i = 0; i < _src->size; ++i)
		{
			_dest->data[i] = _src->data[i] * scale;
		}
	}
	else
	{
		mat_assign(_dest->mat_rep,_src->mat_rep);
	}

	ASSERT(vec_valid(_dest) == VECTOR_OK);
}

void vec_print(vector_t* _v)
{
	ASSERT(vec_valid(_v) == VECTOR_OK);

	vectorScale_t i;
	printf("[");
	for(i = 0; i < _v->size; ++i)
	{
		printf("%f ",_v->data[i]);
	}
	printf("]\n");
}

vectorError_t vec_valid(vector_t* _v)
{
	vectorError_t retValue = VECTOR_OK;
	if(_v == NULL
		|| mat_valid(_v->mat_rep) == MATRIX_FAILED
		|| _v->mat_rep->cols != 1
		|| _v->mat_rep->rows != _v->size
		|| _v->data == NULL)
		retValue = VECTOR_FAILED;

	return retValue;
}

