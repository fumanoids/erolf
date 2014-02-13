/**
 * @file quaternion.h
 *
 * @defgroup quaternion
 * @ingroup mathLibrary
 *
 * @brief Provides a struct,functions and macros to handle data in quaternion representation.
 *
 * Internaly they are stored in a vector3d and a vector4d representation, so every vector
 * operation can be applied on them.
 *
 * @author S. Gene G. gene@staubsaal.de
 *
 * @date 15.11.2012 - created
 *
 * Examples on how to use the code, can be found in test_quaternion.c.
 *
 *@{
 */
#ifndef MATH_QUATERNION_H
#define MATH_QUATERNION_H

#include <stdint.h>
#include "matrix.h"
#include "vector.h"

/** @brief Datatype in which a entry of the quaternion is represented
 */
typedef vectorData_t quaternionData_t;


/** @brief struct which represents a quaternion
 * 
 * It provides a pointer to the scalar/real part and one the imaginary
 * part as 3d vector.
 * It also offers a pointer to a 4d vector representation.
 */
typedef struct tag_quaternion_t
{
	quaternionData_t* const scalar;
	vector_t* const vector3d;
	vector_t* const vector4d;
} quaternion_t;


/** @brief Datatype to represent errors that occur in a function
 */
typedef enum tag_quaternionError {
	QUATERNION_OK,
	QUATERNION_FAILED
} quaternionError_t;


/** @brief Declaring and initializing a quaternion filled with zeros
 *
 * @param[in] name The name of the quaternion
 */
#define QUAT_INIT(name) \
	quaternionData_t _quaternion_buffer_##name[4] = {0.0}; \
	VEC_INIT_DED_BUF(_quaternion_vector3d_##name,3,_quaternion_buffer_##name +1) \
	VEC_INIT_DED_BUF(_quaternion_vector4d_##name,4,_quaternion_buffer_##name) \
	quaternion_t name = {_quaternion_buffer_##name, &_quaternion_vector3d_##name, &_quaternion_vector4d_##name};
#define STATIC_QUAT_INIT(name) \
	static quaternionData_t _quaternion_buffer_##name[4] = {0.0}; \
	STATIC_VEC_INIT_DED_BUF(_quaternion_vector3d_##name,3,_quaternion_buffer_##name +1) \
	STATIC_VEC_INIT_DED_BUF(_quaternion_vector4d_##name,4,_quaternion_buffer_##name) \
	static quaternion_t name = {_quaternion_buffer_##name, &_quaternion_vector3d_##name, &_quaternion_vector4d_##name};


/** @brief Declaring and initializing a quaternion with custom values 
 *
 * @param[in] name The name of the quaternion
 * @param[in] s Scalar part of the quaternion
 * @param[in] v0,v1,v2 Imaginary part of the quaternion
 */
#define QUAT_INIT_SET(name, s, v0, v1, v2) \
	quaternionData_t _quaternion_buffer_##name[4] = {s, v0,v1,v2}; \
	VEC_INIT_DED_BUF(_quaternion_vector3d_##name,3,_quaternion_buffer_##name+1) \
	VEC_INIT_DED_BUF(_quaternion_vector4d_##name,4,_quaternion_buffer_##name) \
	quaternion_t name = {_quaternion_buffer_##name, &_quaternion_vector3d_##name, &_quaternion_vector4d_##name};
#define STATIC_QUAT_INIT_SET(name, s, v0, v1, v2) \
	static quaternionData_t _quaternion_buffer_##name[4] = {s, v0,v1,v2}; \
	STATIC_VEC_INIT_DED_BUF(_quaternion_vector3d_##name,3,_quaternion_buffer_##name+1) \
	STATIC_VEC_INIT_DED_BUF(_quaternion_vector4d_##name,4,_quaternion_buffer_##name) \
	static quaternion_t name = {_quaternion_buffer_##name, &_quaternion_vector3d_##name, &_quaternion_vector4d_##name};


/** @brief Declaring and initializing a quaternion with custom values 
 *
 * Dedicated buffer must provide memory of the size:
 * sizeof(quaternionData_t)*4.
 * It need to be filled with valid values.
 *
 * @param[in] name The name of the quaternion
 * @param[in] buffer Pointer to a buffer where entries are stored
 */
#define QUAT_INIT_DED_BUF(name, buffer) \
	VEC_INIT_DED_BUF(_quaternion_vector3d_##name,3,buffer +1) \
	VEC_INIT_DED_BUF(_quaternion_vector4d_##name,4,buffer) \
	quaternion_t name = {buffer, &_quaternion_vector3d_##name, &_quaternion_vector4d_##name};
#define STATIC_QUAT_INIT_DED_BUF(name, buffer) \
	STATIC_VEC_INIT_DED_BUF(_quaternion_vector3d_##name,3,buffer +1) \
	STATIC_VEC_INIT_DED_BUF(_quaternion_vector4d_##name,4,buffer) \
	static quaternion_t name = {buffer, &_quaternion_vector3d_##name, &_quaternion_vector4d_##name};


/** @brief Assign a quaternion to another
 * 
 * @param[out] _dest Quaternion to assign to
 * @param[in] _src
 */
void quat_assign(quaternion_t* _dest, quaternion_t* _src);


/** @brief Conjugate of a quaternion
 *
 * @param[out] _dest Quaternion to save the conjugate in
 * @param[in] _src Quaternion to conjugate
 */
void quat_conjugate(quaternion_t* _dest, quaternion_t* _src);


/** @brief Compares two Quaternion
 *
 * Entry differences must be smaller then
 * _epsilon to be counted as equal.
 *
 * @param[in] _q1,_q2 QUATERNion to compare
 * @param[in] _epsilon Maximum difference between two entries
 * @return QUATERNION_OK if equal, QUATERNION_FAILED otherwise
 */
quaternionError_t quat_eq(quaternion_t* _q1, quaternion_t* _q2, quaternionData_t _epsilon);


/** @brief Interpolate between two quaterion
 * 
 * The interpolation is made with the so called slerp function.
 * Value _alpha must be between 0 and 1
 *
 * @param[out] _dest Quaternion to save the result of the interpolation
 * @param[in] _q1,_q2 Quaternion pair to interpolate
 * @param[in] _alpha Value similar it should be to _q1 or _q2
 * @param[in] _epsilon Value to avoid underflow problems
 */
void quat_interpolate(quaternion_t* _dest, quaternion_t* _q1, quaternion_t* _q2,quaternionData_t _alpha, quaternionData_t _epsilon);


/** @brief Computes inverse of a quaternion
 *
 * @param[out] _dest Quaternion storing the computed inverse
 * @param[in] _src Quaternion to compute the inverse of
 */
void quat_inv(quaternion_t* _dest, quaternion_t* _src);


/** @brief Multiplicate two quaternion
 * 
 * @param[out] _dest Quaternion to store the product
 * @param[in] _q1,_q2 Quaternion to multiply
 */ 
void quat_mul(quaternion_t* _dest, quaternion_t* _q1, quaternion_t* _q2);


/** @brief Prints a Quaternion to stdout
 */ 
void quat_print( quaternion_t* _q);


/** @brief Rotates a 3d vector around a quaternion
 *
 * @warning vector _dest and _src must be size three and they must
 * not point to the same address.
 *
 * @param[out] _dest Vector to save the rotation to
 * @param[in] _q Quaternion to rotate around
 * @param[in] _src Vector to rotate
 */
void quat_rot_vec(vector_t* _dest, quaternion_t* _q, vector_t* _src);


/** @brief Rotates a 3d vector around the inverse of a quaternion
 *
 * @warning vector _dest and _src must be size three and they must
 * not point to the same address.
 *
 * @param[out] _dest Vector to save the rotation to
 * @param[in] _q Quaternion to rotate around
 * @param[in] _src Vector to rotate
 */
void quat_rotinv_vec(vector_t* _dest, quaternion_t* _q, vector_t* _src);


/** @brief Scale Quaternion
 *
 * @param[out] _dest Quaternion to store the scaled quaternion
 * @param[in] _src Quaternion to scale
 * @param[in] _scale 
 */
void quat_scale(quaternion_t* _dest, quaternion_t* _src, quaternionData_t _scale);


/** @brief Setting quaternion to a specific orientation
 *
 * Will set a quaternion to a certain rotation arround the specified axe.
 *
 * @param[out] _dest Quaternion to modify
 * @param[in] _angle Angle in radian 
 * @param[in] _axis Axe to rotate around
 */
void quat_set_rot(quaternion_t* _dest, quaternionData_t _angle, vector_t* _axis);


/** @brief Convert quaternion to a 3x3 rotation matrix
 *
 * The matrix must be a 3x3 matrix.
 *
 * @param[out] _dest Matrix to save the quaternion to
 * @param[in] _src Quaternion to convert
 */
void quat_to_mat(matrix_t* _dest, quaternion_t* _src);


/** @brief Checks if a quaternion is valid
 * 
 * Will check if the quaternion structur is filled with valid pointers.
 * It will also check if all values are not NaN and not Inf
 * @param[in] _q quaternion to check
 * @return QUATERNION_OK if vector is valid, QUATERNION_FAILED otherwise
 */
quaternionError_t quat_valid(quaternion_t* _q);


//@}
#endif
