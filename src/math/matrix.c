#include "matrix.h"
#include "mathOptions.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>




void mat_add(matrix_t* _dest, matrix_t* _m1, matrix_t* _m2)
{
	ASSERT(mat_valid(_dest) == MATRIX_OK);
	ASSERT(mat_valid(_m1) == MATRIX_OK);
	ASSERT(mat_valid(_m2) == MATRIX_OK);
	ASSERT(mat_sizematch(_dest,_m1) == MATRIX_OK); 
	ASSERT(mat_sizematch(_m1, _m2) == MATRIX_OK);

	matrixScale_t i;
	for(i = 0; i < _dest->cols * _dest->rows; ++i)
	{
		_dest->data[i] = _m1->data[i] + _m2->data[i];
	}

	ASSERT(mat_valid(_dest) == MATRIX_OK);
}
void mat_adjugate(matrix_t* _dest, matrix_t* _src, matrix_t* _tmp)
{
	ASSERT(mat_valid(_dest) == MATRIX_OK);
	ASSERT(mat_valid(_src) == MATRIX_OK);
	ASSERT(mat_valid(_tmp) == MATRIX_OK);

	ASSERT(mat_sizematch(_dest,_src) == MATRIX_OK);
	ASSERT(_dest->cols == _dest->rows);
	ASSERT(_tmp->cols == _dest->cols - 1);
	ASSERT(_tmp->rows == _dest->rows - 1);
	
	if(_dest->cols > 1)
	{
		matrixScale_t i,j,ct;
		ct = 0;
		for(i = 0; i < _dest->rows; ++i)
		{
			for(j = 0; j < _dest->cols; ++j)
			{
				matrixData_t sign = (i%2*2-1) * (j%2*2-1);
				mat_minor(_tmp,_src,i,j);
				matrixData_t x = mat_det(_tmp,_tmp);
				
				_dest->data[i*_dest->cols + j] = sign * x;


				++ct;
			}
		}
	}
	else
	{
		_dest->data[0] = _src->data[0];
	}

	ASSERT(mat_valid(_dest) == MATRIX_OK);
}

void mat_assign(matrix_t* _dest, matrix_t* _src)
{
	ASSERT(mat_valid(_dest) == MATRIX_OK);
	ASSERT(mat_valid(_src) == MATRIX_OK);
	ASSERT(mat_sizematch(_dest,_src) == MATRIX_OK);

	matrixScale_t i;
	for(i = 0; i < _dest->cols * _dest->rows; ++i)
	{
		_dest->data[i] = _src->data[i];
	}

	ASSERT(mat_valid(_dest) == MATRIX_OK);
}

matrixData_t mat_det( matrix_t* _m, matrix_t* _tmp)
{
	ASSERT(mat_valid(_m) == MATRIX_OK);
	ASSERT(mat_valid(_tmp) == MATRIX_OK);
	ASSERT(_m->cols == _m->rows);
	ASSERT(mat_sizematch(_m,_tmp) == MATRIX_OK);

	matrixScale_t n = _m->rows;

	matrixData_t retValue = 1.0f;

	if(mat_luDecomposition(_tmp,_m) == MATRIX_FAILED)
	{
		retValue = 0.0f;
	}
	else
	{
		matrixScale_t i;
		for(i = 0; i < n; ++i)
		{
			retValue *= _tmp->data[i*n+i];
		}
	}

	return retValue;
}
matrixError_t mat_eq( matrix_t* _m1, matrix_t* _m2, matrixData_t _epsilon)
{
	ASSERT(mat_valid(_m1) == MATRIX_OK);
	ASSERT(mat_valid(_m2) == MATRIX_OK);
	ASSERT(mat_sizematch(_m1,_m2) == MATRIX_OK);
	
	matrixScale_t i;

	matrixError_t retValue = MATRIX_OK;
	for(i = 0; i < _m1->cols * _m1->rows && retValue == MATRIX_OK; ++i)
	{
		matrixData_t diff = fabsf((_m1->data[i] - _m2->data[i]));
		if(diff >_epsilon)
			retValue = MATRIX_FAILED;
	}

	return retValue;
}

void mat_eye(matrix_t* _dest)
{
	ASSERT(mat_valid(_dest) == MATRIX_OK);
	ASSERT(_dest->rows == _dest->cols);

	mat_zero(_dest);

	matrixScale_t i;
	for(i = 0; i < _dest->rows; ++i)
	{
		_dest->data[i*_dest->cols + i] = 1.0f;
	}

	ASSERT(mat_valid(_dest) == MATRIX_OK);
}

matrixData_t mat_get( matrix_t* _m, matrixScale_t _row, matrixScale_t _col)
{
	ASSERT(mat_valid(_m) == MATRIX_OK);
	ASSERT(_row < _m->rows);
	ASSERT(_col < _m->cols);

	return _m->data[_row * _m->cols + _col];
}
matrixError_t mat_inv(matrix_t* _dest, matrix_t* _src, matrix_t* _tmp1, matrix_t* _tmp2)
{
	ASSERT(mat_valid(_dest) == MATRIX_OK);
	ASSERT(mat_valid(_src) == MATRIX_OK);
	ASSERT(mat_valid(_tmp1) == MATRIX_OK);
	ASSERT(mat_valid(_tmp2) == MATRIX_OK);

	ASSERT(mat_sizematch(_dest,_src) == MATRIX_OK);
	ASSERT(mat_sizematch(_dest,_tmp1) == MATRIX_OK);
	ASSERT(_src->rows == _src->cols);
	ASSERT(_dest != _src);
	ASSERT(_tmp2->rows == _dest->rows - 1);
	ASSERT(_tmp2->cols == _dest->cols - 1);

	matrixError_t retValue = MATRIX_OK;

	if(_src->rows == 1)
	{
		if(_src->data[0] == 0.0f)
			retValue = MATRIX_FAILED;
		else
			_dest->data[0] = 1/_src->data[0];
	}
	else
	{

		matrixData_t det = mat_det(_src, _tmp1);

		if(det == 0.0f)
			retValue = MATRIX_FAILED;
		else
		{
			matrixData_t scale = 1.0f/det;
			mat_adjugate(_tmp1,_src,_tmp2);
			mat_transpose(_dest,_tmp1);
			mat_scale(_dest,_dest,scale);
		}
	}

	ASSERT(mat_valid(_dest) == MATRIX_OK);
	return retValue;
}
matrixError_t mat_luDecomposition(matrix_t* _dest, matrix_t* _src)
{
	ASSERT(mat_valid(_dest) == MATRIX_OK);
	ASSERT(mat_valid(_src) == MATRIX_OK);
	ASSERT(mat_sizematch(_dest,_src) == MATRIX_OK);

	mat_assign(_dest,_src);
	matrixScale_t n = _src->rows;

	matrixScale_t i,j,k;
	matrixError_t retValue = MATRIX_OK;
	for(i = 0; i < n; ++i)
	{
		for(j = i; j < n; ++j)
		{
			for(k = 0; k <= i-1; ++k)
			{
				_dest->data[i*n + j] -= _dest->data[i*n + k] * _dest->data[k*n + j];
			}
		}
		for(j = i+1; j < n && retValue == MATRIX_OK; ++j)
		{
			for(k = 0; k <= i-1 && retValue == MATRIX_OK; ++k)
			{
				_dest->data[j*n + i] -= _dest->data[j*n + k] * _dest->data[k*n + i];
			}
			if(_dest->data[i*n +i] == 0.0f)
				retValue = MATRIX_FAILED;
			else
				_dest->data[j*n + i] /= _dest->data[i*n + i];
		}
	}

	ASSERT(mat_valid(_dest) == MATRIX_OK);
	return retValue;
}

void mat_minor(matrix_t* _dest, matrix_t* _src, matrixScale_t _row, matrixScale_t _col)
{
	ASSERT(mat_valid(_dest) == MATRIX_OK);
	ASSERT(mat_valid(_src) == MATRIX_OK);

	ASSERT(_src != _dest);
	ASSERT(_src->cols == _dest->cols+1);
	ASSERT(_src->rows == _dest->rows+1);
	ASSERT(_src->cols > _col);
	ASSERT(_src->rows > _row);
		
	matrixScale_t i,j,ct;
	ct = 0;
	for(i = 0; i < _src->rows; ++i)
	{
		for(j = 0; j < _src->cols; ++j)
		{
			if(i == _row || j == _col) continue;
			_dest->data[ct++] = _src->data[i*_src->cols + j];
		}
	}

	ASSERT(mat_valid(_dest) == MATRIX_OK);
}


void mat_mul(matrix_t* _dest, matrix_t* _m1, matrix_t* _m2)
{
	ASSERT(mat_valid(_dest) == MATRIX_OK);
	ASSERT(mat_valid(_m1) == MATRIX_OK);
	ASSERT(mat_valid(_m2) == MATRIX_OK);
	ASSERT(_m1 != _dest && _m2 != _dest);
	ASSERT(_m1->cols == _m2->rows);
	ASSERT(_m1->rows == _dest->rows);
	ASSERT(_m2->cols == _dest->cols);

	matrixScale_t i,j,k;
	for(i = 0; i < _dest->cols; ++i)
	{
		for(j = 0; j < _dest->rows; ++j)
		{
			matrixData_t sum = 0.0f;
			for(k = 0; k < _m1->cols; ++k)
			{
				sum += _m1->data[j*_m1->cols + k] * _m2->data[k*_m2->cols + i];
			}
			_dest->data[j * _dest->cols + i] = sum;
		}
	}

	ASSERT(mat_valid(_dest) == MATRIX_OK);
}
void mat_ones(matrix_t* _dest)
{
	ASSERT(mat_valid(_dest) == MATRIX_OK);

	matrixScale_t i,j;
	for(i = 0; i < _dest->rows; ++i)
	{
		for(j = 0; j < _dest->cols; ++j)
		{
			_dest->data[i*_dest->cols + j] = 1.0f;
		}
	}
	ASSERT(mat_valid(_dest) == MATRIX_OK);
}


void mat_print( matrix_t* _m)
{
	ASSERT(mat_valid(_m) == MATRIX_OK);

	matrixScale_t i,j;
	printf("[\n");
	for(i = 0; i < _m->rows; ++i)
	{
		printf("[ ");
		for(j = 0; j < _m->cols; ++j)
		{
			printf("%f ",_m->data[i*_m->cols+j]);
		}
		printf("]\n");
	}
	printf("]\n");
}
void mat_rand(matrix_t* _m)
{
	ASSERT(mat_valid(_m) == MATRIX_OK);

	matrixScale_t i;
	for(i = 0; i < _m->cols * _m->rows; ++i)
	{
		_m->data[i] = (matrixData_t)rand()/RAND_MAX;
/*		matrixData_t rd = (matrixData_t)rand() / (matrixData_t)RAND_MAX;
		rd = (rd-0.5) * 50; //sigma 0.02
		_m->data[i] = exp(-rd * rd*0.5);*/
	}

	ASSERT(mat_valid(_m) == MATRIX_OK);
}
#define NSUM 30
matrixData_t _rand(void)
{
	matrixData_t x = 0;
	matrixScale_t i;
	for(i = 0; i < NSUM; i++)
		x += (matrixData_t)rand()/RAND_MAX;
	x -= NSUM * 0.5;
	x /= sqrt(NSUM / 12.0);

	return x;
}
void mat_randgauss(matrix_t* _m, matrixData_t _scale)
{
	//ASSERT(mat_valid(_m) == MATRIX_OK);

	matrixScale_t i;
	for(i = 0; i < _m->cols * _m->rows; ++i)
	{
		_m->data[i] = _rand() * _scale;
	}

	ASSERT(mat_valid(_m) == MATRIX_OK);

}


void mat_scale(matrix_t* _dest, matrix_t* _src, matrixData_t _scale)
{
	ASSERT(mat_valid(_dest) == MATRIX_OK);
	ASSERT(mat_valid(_src) == MATRIX_OK);
	ASSERT(mat_sizematch(_dest,_src) == MATRIX_OK);
	ASSERT(isfinite(_scale) != 0);

	matrixScale_t i;
	for(i = 0; i < _dest->cols * _dest->rows; ++i)
	{
		_dest->data[i] = _src->data[i] * _scale;
	}

	ASSERT(mat_valid(_dest) == MATRIX_OK);
}

void mat_set( matrix_t* _m, matrixScale_t _row, matrixScale_t _col, matrixData_t _value)
{
	ASSERT(mat_valid(_m) == MATRIX_OK);
	ASSERT(_row < _m->rows);
	ASSERT(_col < _m->cols);
	ASSERT(isfinite(_value) != 0);

	_m->data[_row * _m->cols + _col] = _value;

	ASSERT(mat_valid(_m) == MATRIX_OK);
}
matrixError_t mat_sizematch(matrix_t* _m1, matrix_t* _m2)
{
	ASSERT(mat_valid(_m1) == MATRIX_OK);
	ASSERT(mat_valid(_m2) == MATRIX_OK);

	matrixError_t retValue = MATRIX_OK;
	if(_m1->cols != _m2->cols
		||_m1->rows != _m2->rows)
		retValue = MATRIX_FAILED;
	return retValue;
}
void mat_sub(matrix_t* _dest, matrix_t* _m1, matrix_t* _m2)
{
	ASSERT(mat_valid(_dest) == MATRIX_OK);
	ASSERT(mat_valid(_m1) == MATRIX_OK);
	ASSERT(mat_valid(_m2) == MATRIX_OK);
	ASSERT(mat_sizematch(_dest,_m1) == MATRIX_OK);
	ASSERT(mat_sizematch(_m1, _m2) == MATRIX_OK);
	
	matrixScale_t i;
	for(i = 0; i < _dest->cols * _dest->rows; ++i)
	{
		_dest->data[i] = _m1->data[i] - _m2->data[i];
	}

	ASSERT(mat_valid(_dest) == MATRIX_OK);
}


void mat_transpose(matrix_t* _dest, matrix_t* _src)
{
	ASSERT(mat_valid(_dest) == MATRIX_OK);
	ASSERT(mat_valid(_src) == MATRIX_OK);
	ASSERT(_dest != _src);
	ASSERT(_dest->cols == _src->rows);
	ASSERT(_dest->rows == _src->cols);

	matrixScale_t i,j;
	for(i = 0; i < _src->rows; ++i)
	{
		for(j = 0; j < _src->cols; ++j)
		{
			_dest->data[j*_dest->cols + i] = _src->data[i * _src->cols + j];
		}
	}

	ASSERT(mat_valid(_dest) == MATRIX_OK);
}
matrixError_t mat_valid( matrix_t* _m)
{
	matrixError_t returnValue = MATRIX_OK;
	if(_m == NULL
		|| _m->data == NULL)
	{
		returnValue = MATRIX_FAILED;
	}
	else
	{
		matrixScale_t i,j;
		for(i = 0; i < _m->rows && returnValue == MATRIX_OK; ++i)
		{
			for(j = 0; j < _m->cols && returnValue == MATRIX_OK; ++j)
			{
				if(isfinite(_m->data[i*_m->cols + j]) == 0)
				{
					returnValue = MATRIX_FAILED;
				}
			}
		}
	}
	return returnValue;
}

void mat_zero(matrix_t* _dest)
{
	ASSERT(mat_valid(_dest) == MATRIX_OK);

	matrixScale_t i,j;
	for(i = 0; i < _dest->rows; ++i)
	{
		for(j = 0; j < _dest->cols; ++j)
		{
			_dest->data[i*_dest->cols + j] = 0.0f;
		}
	}
	ASSERT(mat_valid(_dest) == MATRIX_OK);
}

