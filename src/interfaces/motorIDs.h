/*
 * motorIDs.h
 *
 *  Created on: Mar 7, 2013
 *      Author: lutz
 */

#ifndef MOTORIDS_H_
#define MOTORIDS_H_


enum {
	MOTOR_NO_MOTOR              =  0,

	MOTOR_FIRST_MOTOR           =  1,

	// upper body
	MOTOR_HEAD_PITCH            =  1,
	MOTOR_HEAD_TURN             =  2,

	//arms
	MOTOR_LEFT_ARM_PITCH        =  3,
	MOTOR_RIGHT_ARM_PITCH       =  4,
	MOTOR_LEFT_ARM_ROLL         =  5,
	MOTOR_RIGHT_ARM_ROLL        =  6,
	MOTOR_LEFT_ELBOW            =  7,
	MOTOR_RIGHT_ELBOW           =  8,

	// legs
	MOTOR_STOMACH               =  9,
	MOTOR_SPINE                 = 10,

	MOTOR_LEFT_HIP_ROLL         = 11,
	MOTOR_RIGHT_HIP_ROLL        = 12,
	MOTOR_LEFT_KNEE_TOP         = 13,
	MOTOR_RIGHT_KNEE_TOP        = 14,
	MOTOR_LEFT_KNEE_BOTTOM      = 15,
	MOTOR_RIGHT_KNEE_BOTTOM     = 16,
	MOTOR_LEFT_FOOT_ROLL        = 17,
	MOTOR_RIGHT_FOOT_ROLL       = 18,
	MOTOR_LEFT_FOOT_YAW         = 19,
	MOTOR_RIGHT_FOOT_YAW        = 20,

	// misc
	MOTOR_LAST_MOTOR            = 20, // make sure to update to highest motor ID!
	MOTOR_END_OF_LIST           = MOTOR_LAST_MOTOR + 1,
	MOTOR_ALL_MOTORS            = 0xFE,

	// three-wheeled robot
	WHEEL_LEFT                  = 30,
	WHEEL_BACK                  = 31,
	WHEEL_RIGHT                 = 32,

	GYRO                        = 101
};

#define DYNAMIXEL_LAST_TORSO_MOTOR MOTOR_RIGHT_ELBOW
#define DYNAMIXEL_LAST_RIGHT_LEG_MOTOR MOTOR_RIGHT_FOOT_YAW
#define DYNAMIXEL_LAST_LEFT_LEG_MOTOR MOTOR_LEFT_FOOT_YAW

typedef uint8_t motorID_t;

#endif /* MOTORIDS_H_ */
