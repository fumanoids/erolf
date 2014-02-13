/*
 * L3GD20.h
 *
 *  Created on: Nov 28, 2012
 *      Author: lutz
 */

#ifndef L3GD20_H_
#define L3GD20_H_

#include <flawless/stdtypes.h>

/*
 * raw data produced by this module.
 * Those data is handled internally but you can register on that message as well if you are interested in it
 */

typedef int16_t l3gd20_gyroDataRaw_t[3];


#define L3GD20_DEVICE_ADRESS (0xd6)

#define L3GD20_REGISTER_WHOAMI     0x0f

#define L3GD20_REGISTER_CTRL_REG1  0x20
#define L3GD20_REGISTER_CTRL_REG2  0x21
#define L3GD20_REGISTER_CTRL_REG3  0x22
#define L3GD20_REGISTER_CTRL_REG4  0x23
#define L3GD20_REGISTER_CTRL_REG5  0x24

#define L3GD20_REGISTER_REFERENCE  0x25

#define L3GD20_REGISTER_OUT_TEMP   0x26

#define L3GD20_REGISTER_STATUS_REG 0x0f

#define L3GD20_REGISTER_OUT_X_L    0x28
#define L3GD20_REGISTER_OUT_X_H    0x29
#define L3GD20_REGISTER_OUT_Y_L    0x2a
#define L3GD20_REGISTER_OUT_Y_H    0x2b
#define L3GD20_REGISTER_OUT_Z_L    0x2c
#define L3GD20_REGISTER_OUT_Z_H    0x2d


#define L3GD20_FIFO_CTRL_REG       0x28
#define L3GD20_FIFO_SRC_REG        0x28


#define L3GD20_INT1_CFG            0x28
#define L3GD20_INT1_SRC            0x28
#define L3GD20_INT1_TSH_XH         0x32
#define L3GD20_INT1_TSH_XL         0x33
#define L3GD20_INT1_TSH_YH         0x34
#define L3GD20_INT1_TSH_YL         0x35
#define L3GD20_INT1_TSH_ZH         0x36
#define L3GD20_INT1_TSH_ZL         0x37

#define L3GD20_INT1_DURATION       0x32

void L3GD20_setupSensor(void);

#endif /* L3GD20_H_ */
