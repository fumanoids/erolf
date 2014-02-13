/**
 * @file vector.h
 *
 * @defgroup vector
 * @ingroup mathLibrary
 *
 * @brief Provides a struct,functions and macros to handle data in vector representation.
 *
 * Internaly they are stored in matrix representation, so every matrix
 * operation can be applied on them.
 *
 * @author S. Gene G. gene@staubsaal.de
 *
 * @date 15.11.2012 - created
 *
 * Examples on how to use the code, can be found in test_vector.c.
 *
 *@{
 */
#ifndef MATH_VECTOR_H
#define MATH_VECTOR_H

#include <stdint.h> 
#include "matrix.h"

/** @brief Datatype in which the size is stored
 * */
typedef matrixScale_t vectorScale_t;


/** @brief Datatype in which a entry of the vector is represented
 */
typedef matrixData_t vectorData_t;


/** @brief struct which represents a vector
 * 
 * It provides information about the number of entries the
 * vector has and where the entries of the vectors are stored.
 * It also offers a pointer to a matrix representation of the vector
 */
typedef struct tag_vector_t
{
	matrix_t* const mat_rep;
	vectorScale_t const size;
	vectorData_t* const data;
} vector_t;


/** @brief Datatype to represent errors that occur in a function
 */
typedef enum tag_vectorError {
	VECTOR_OK,
	VECTOR_FAILED
} vectorError_t;


/** @brief Declaring and initializing a vector filled with zeros
 *
 * Vector must have at least a size of one.
 * 
 * @param[in] name The name of the vector
 * @param[in] size Number of entries
 */
#define VEC_INIT(name,size) \
	vectorData_t _vector_data_##name[size] = {0.0f}; \
	MAT_INIT_DED_BUF(_vector_mat_rep_##name,size,1,_vector_data_##name) \
	vector_t name = {&_vector_mat_rep_##name, size, _vector_data_##name };
#define STATIC_VEC_INIT(name,size) \
	static vectorData_t _vector_data_##name[size] = {0.0f}; \
	STATIC_MAT_INIT_DED_BUF(_vector_mat_rep_##name,size,1,_vector_data_##name) \
	static vector_t name = {&_vector_mat_rep_##name, size, _vector_data_##name };


/** @brief Declaring and initializing a vector with custom values.
 *
 * Vector must have at least a size of one.
 * It will be filled beginning from the top to the bottom
 *
 * @param[in] name The name of the vector
 * @param[in] size Number of entries
 * @param[in] ... Values to fill the vector
 */
#define VEC_INIT_SET(name,size, ...) \
	vectorData_t _vector_data_##name[size] = {__VA_ARGS__}; \
	MAT_INIT_DED_BUF(_vector_mat_rep_##name,size,1,_vector_data_##name) \
	vector_t name = {&_vector_mat_rep_##name, size, _vector_data_##name };
#define STATIC_VEC_INIT_SET(name,size, ...) \
	static vectorData_t _vector_data_##name[size] = {__VA_ARGS__}; \
	STATIC_MAT_INIT_DED_BUF(_vector_mat_rep_##name,size,1,_vector_data_##name) \
	static vector_t name = {&_vector_mat_rep_##name, size, _vector_data_##name };


 /** @brief Declaring a vector with a dedicated buffer
 *
 * Vector must have at least a size of one.
 * Dedicated buffer must provide memory of the size:
 * sizeof(vectorData_t)*size.
 * It need to be filled with valid values.
 *
 * @param[in] name The name of the vector
 * @param[in] size Number of entries
 * @param[in] buffer Pointer to a buffer where entries are stored
 */
#define VEC_INIT_DED_BUF(name,size,buffer) \
	MAT_INIT_DED_BUF(_vector_mat_rep_##name,size,1,buffer) \
	vector_t name = {&_vector_mat_rep_##name, size, buffer };
#define STATIC_VEC_INIT_DED_BUF(name,size,buffer) \
	STATIC_MAT_INIT_DED_BUF(_vector_mat_rep_##name,size,1,buffer) \
	static vector_t name = {&_vector_mat_rep_##name, size, buffer };


/** @brief Computes the square length(norm)
 * 
 * @param _v Vector to compute the square length from
 * @return the square length
 */
vectorData_t vec_length_sqr(vector_t* _v);


/** @brief Computes the cross product
 *
 * @todo only works with vectors of size 1 and 3
 *
 * Vectors _dest, _v1 and _v2 must not point to the same address.
 * They need to be of the same size.
 *
 * @param[out] _dest Vector to store the result in
 * @param[in] _v1, _v2 Vectors to compute the cross product from
 */
void vec_mul_cross(vector_t* _dest, vector_t* _v1, vector_t* _v2);


/** @brief Computes the scalar product
 * 
 * Vectors _v1 and _v2 need to be of the same size.
 * @param[in] _v1,_v2 Vectors using for computation
 * @return the scalar product
 */
vectorData_t vec_mul_dot(vector_t* _v1, vector_t* _v2);


/** @brief normalize vector
 *
 * Vectors _dest and _src must have same size.
 *
 * @param[out] _dest Vector to store the normal in
 * @param[in] _src Vector to normalize
 */
void vec_normalize(vector_t* _dest, vector_t* _src);


/** @brief Prints the vector to stdout
 */ 
void vec_print(vector_t* _v);


/** @brief Checks if a vector is valid
 * 
 * Will check if the vector structur is filled with valid pointers.
 * It will also check if all values are not NaN and not Inf
 * @param[in] _v vector to check
 * @return VECTOR_OK if vector is valid, VECTOR_FAILED otherwise
 */
vectorError_t vec_valid(vector_t* _v);


//@}
#endif
