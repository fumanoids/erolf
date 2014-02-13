/**
 * @file matrix.h
 *
 * @defgroup matrix
 * @ingroup mathLibrary
 *
 * @brief Provides a struct,functions and macros to handle data in matrix representation.
 *
 * @author S. Gene G. gene@staubsaal.de
 *
 * @date 14.11.2012 - created
 *
 * Examples on how to use the code, can be found in test_matrix.c.
 *
 *@{
 */
#ifndef MATH_MATRIX_H
#define MATH_MATRIX_H

#include <stdint.h> 

/** @brief Datatype in which rows and columns are counted
 */
typedef uint16_t matrixScale_t;


/** @brief Datatype in which a entry of the matrix is represented
 */
typedef float matrixData_t;


/** @brief struct which represents a matrix
 * 
 * It provides information about the number of rows and columns the
 * matrix has and where the entries of the matrix are stored
 */
typedef struct tag_matrix_t
{
	matrixScale_t const rows;
	matrixScale_t const cols;
	matrixData_t* const data;
} matrix_t;


/** @brief Datatype to represent errors that occur in a function
 */
typedef enum tag_matrixError {
	MATRIX_OK,
	MATRIX_FAILED
} matrixError_t;


/** @brief Declaring and initializing a matrix filled with zeros
 *
 * Matrix must have at least dimension 1x1.
 *
 * @param[in] name The name of the matrix
 * @param[in] rows,cols Number of rows and columns
 */
#define MAT_INIT(name,rows,cols) \
	matrixData_t _matrix_data_##name[(rows) * (cols)] = {0.0}; \
	matrix_t name = {rows, cols, _matrix_data_##name};
#define STATIC_MAT_INIT(name,rows,cols) \
	static matrixData_t _matrix_data_##name[(rows) * (cols)] = {0.0}; \
	static matrix_t name = {rows, cols, _matrix_data_##name};


/** @brief Declaring and initializing a matrix with custom values.
 *
 * Matrix must have at least dimension 1x1.
 * It will be filled row by row, from left to right
 *
 * @param[in] name The name of the matrix
 * @param[in] rows,cols Number of rows and columns
 * @param[in] ... Values to fill the matrix
 */
#define MAT_INIT_SET(name,rows,cols, ...) \
	matrixData_t _matrix_buffer_##name[(rows)*(cols)] = {__VA_ARGS__}; \
	matrix_t name = {rows, cols, _matrix_buffer_##name};
#define STATIC_MAT_INIT_SET(name,rows,cols, ...) \
	static matrixData_t _matrix_buffer_##name[(rows)*(cols)] = {__VA_ARGS__}; \
	static matrix_t name = {rows, cols, _matrix_buffer_##name};


/** @brief Declaring a matrix with a dedicated buffer
 *
 * Matrix must have at least dimension 1x1.
 * Dedicated buffer must provide memory of the size:
 * sizeof(matrixData_t)*rows*cols.
 * It need to be filled with valid values.
 * 
 * @param[in] name The name of the matrix
 * @param[in] rows,cols Number of rows and columns
 * @param[in] buffer Pointer to a buffer where entries are stored
 */
#define MAT_INIT_DED_BUF(name,rows,cols, buffer) \
	matrix_t name = {rows, cols, buffer};
#define STATIC_MAT_INIT_DED_BUF(name,rows,cols, buffer) \
	static matrix_t name = {rows, cols, buffer};


/** @brief Add two matricies
 *
 * @param[out] _dest Matrix to store the result
 * @param[in] _m1,_m2 Matricies to add
 */
void mat_add(matrix_t* _dest, matrix_t* _m1, matrix_t* _m2);


/** @brief Compute the adjugate of a matrix
 *
 * The Matrix _dest must be a square matrix and have the same dimension as _src.
 * The helper Matrix _tmp must be by one smaller in dimension compared to
 * _dest.
 * @warning _dest, _tmp and _src must not point to the same address
 *
 * @param[out] _dest Matrix to store the result
 * @param[in] _src Matrix to compute the adjugate of
 * @param _tmp Matrix helping computing
 */
void mat_adjugate(matrix_t* _dest, matrix_t* _src, matrix_t* _tmp);


/** @brief Assign a matrix to another
 * 
 * Matricies must have the same dimensions.
 * 
 * @param[out] _dest Matrix to assign to
 * @param[in] _src
 */
void mat_assign(matrix_t* _dest, matrix_t* _src);


/** @brief Computes the determinant of a matrix
 *
 * The matrix _tmp must have the same dimension as _m.
 * Both need to be square matricies.
 *
 * @param[in] _m Computing the determinant of this matrix
 * @param _tmp Matrix helping computing
 * @return determinant
 */
matrixData_t mat_det(matrix_t* _m, matrix_t* _tmp);


/** @brief Compares two matricies
 *
 * Compares two matricies. Entry differences must be smaller then
 * _epsilon to be counted as equal.
 * The matricies _m1 and _m2 must have the same dimensions.
 *
 * @param[in] _m1,_m2 Matricies to compare
 * @param[in] _epsilon Maximum difference between two entries
 * @return MATRIX_OK if equal, MATRIX_FAILED otherwise
 */
matrixError_t mat_eq( matrix_t* _m1, matrix_t* _m2, matrixData_t _epsilon);


/** @brief Sets diagonal to ones.
 *
 * @param[out] _dest Must be a square matrix
 */
void mat_eye(matrix_t* _dest);


/** @brief Read an entry from a matrix
 * 
 * @warning Indicies start with 0.
 *
 * @param[in] _m Matrix to read the entry from
 * @param[in] _row,_col Specifies the entry to read
 * @return The value of the entry
 */
matrixData_t mat_get( matrix_t* _m, matrixScale_t _row, matrixScale_t _col);


/** @brief Computes inverse of a matrix
 *
 *
 * All matricies must be square matricies.
 * The Matricies _dest,_src and _tmp1 must be of same dimension, while
 * _tmp2 is one dimension smaller.
 *
 * @warning _dest,_src,_tmp1,_tmp2 must not point to the same address.
 *
 * @param[out] _dest Matrix storing the computed inverse
 * @param[in] _src Matrix to compute the inverse of
 * @param _tmp1 Matrix helping computing
 * @param _tmp2 Matrix helping computing
 * @return MATRIX_OK if successfull inverted, MATRIX_FAILED otherwise
 */
matrixError_t mat_inv(matrix_t* _dest, matrix_t* _src, matrix_t* _tmp1, matrix_t* _tmp2);


/** @brief Computes a LU decompensation of a matrix
 * 
 * Matrix _dest and _src must have same dimensions.
 *
 * @param[out] _dest Matrix storing the LU decompensation
 * @param[in] _src Matrix to decompensate to L and R
 * @return MATRIX_OK if successfull decomposed, MATRIX_FAILED otherwise
 */
matrixError_t mat_luDecomposition(matrix_t* _dest, matrix_t* _src);


/** @brief Computes the minor of a matrix
 *
 * @warning Indicies start with 0.
 *
 * The matrix _dest must be one dimension smaller then _src
 * @param[out] _dest Matrix to store the minor
 * @param[in]  _src Matrix which to compute the minor of
 * @param[in] _row,_col Row and column to remove
 */
void mat_minor(matrix_t* _dest, matrix_t* _src, matrixScale_t _row, matrixScale_t _col);


/** @brief Multiplicate two matricies
 * 
 * Multiplicate two matricies. The Columns of _m1 must match the rows of
 * _m2. The product of _m1 and _m2 must fit into _dest.
 *
 * @param[out] _dest Matrix to store the product
 * @param[in] _m1,_m2 Matricies to multiply
 */ 
void mat_mul(matrix_t* _dest, matrix_t* _m1, matrix_t* _m2);

/** @brief Filles matrix with ones.
 *
 * @param[out] _dest Matrix to fill with ones
 */
void mat_ones(matrix_t* _dest);


/** @brief Prints the matrix to stdout
 */ 
void mat_print(matrix_t* _m);


/** @brief Fills matrix with random numbers
 * 
 * Every entry will get a value between 0 and 1.
 *
 * @warning This function uses the rand() function from the standard library.
 *
 * @param[out] _m Matrix to fill with random numbers
 */
void mat_rand(matrix_t* _m);

/** @brief Fills matrix with random numbers
 * 
 * Every entry will get a value between lowerBound and upperBound.
 *
 * @warning This function uses the rand() function from the standard library.
 *
 * @param[out] _m Matrix to fill with random numbers
 */
void mat_randgauss(matrix_t* _m, matrixData_t _scale);


/** @brief Scale matrix
 *
 * Matricies _dest and _src must have same dimensions.
 *
 * @param[out] _dest Matrix to store the scaled matrix
 * @param[in] _src Matrix to scale
 * @param[in] _scale 
 */
void mat_scale(matrix_t* _dest, matrix_t* _src, matrixData_t _scale);


/** @brief Set an entry from a matrix
 * 
 * @warning Indicies start with 0.
 *
 * @param[in,out] _m Matrix to set the entry to
 * @param[in] _row,_col Specifies the entry to write
 * @param[in] _value The value that should be set
 */
void mat_set( matrix_t* _m, matrixScale_t _row, matrixScale_t _col, matrixData_t _value);


/** @brief Compares the dimension of two matricies
 *
 * @param[in] _m1,_m2 Matricies to compare
 * @return MATRIX_OK if dimension equals, MATRIX_FAILED otherwise
 */
matrixError_t mat_sizematch( matrix_t* _m1, matrix_t* _m2);


/** @brief Substracting two matricies
 *
 * Matricies _dest,_m1 and _m2 must have same dimensions
 * @param[out] _dest Matrix to store the result
 * @param[in] _m1,_m2 Matricies for the substraction
 */
void mat_sub(matrix_t* _dest, matrix_t* _m1, matrix_t* _m2);


/** @brief Transposes a matrix
 *
 * The matricies _dest and _src must have swaped dimensions.
 *
 * @param[out] _dest Matrix to store the transpose in
 * @param[in] _src Matrix to transpose
 */
void mat_transpose(matrix_t* _dest, matrix_t* _src);


/** @brief Checks if a matrix is valid
 * 
 * Will check if a matrix structur is filled with valid pointers.
 * It will also check if all values are not NaN and not Inf
 * @param[in] _m Matrix to check
 * @return MATRIX_OK if matrix is valid, MATRIX_FAILED otherwise
 */
matrixError_t mat_valid(matrix_t* _m);


/** @brief Filles matrix with zeros.
 *
 * @param[out] _dest Matrix to fill with zeros
 */
void mat_zero(matrix_t* _dest);


//@}
#endif
