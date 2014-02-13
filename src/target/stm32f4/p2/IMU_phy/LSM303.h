/*
 * LSM303.h
 *
 *  Created on: Nov 30, 2012
 *      Author: lutz
 */

#ifndef LSM303_H_
#define LSM303_H_

#include <flawless/stdtypes.h>


/*
 * raw data produced by this module.
 * Those data is handled internally but you can register on that message as well if you are interested in it
 */

typedef int16_t lsm303_magDataRaw_t[3];
typedef int16_t lsm303_accDataRaw_t[3];

#define LSM303_ACC_DEVICE_ADDRESS (0x32)
#define LSM303_MAG_DEVICE_ADDRESS (0x3C)

/*
 * Everything related to the ACC subdevice
 */

#define LSM303_ACC_REG_CTRL_REG1 (0x20)
#define LSM303_ACC_REG_CTRL_REG2 (0x21)
#define LSM303_ACC_REG_CTRL_REG3 (0x22)
#define LSM303_ACC_REG_CTRL_REG4 (0x23)
#define LSM303_ACC_REG_CTRL_REG5 (0x24)
#define LSM303_ACC_REG_CTRL_REG6 (0x25)

#define LSM303_ACC_REFERENCE     (0x26)
#define LSM303_ACC_STATUS_REG_A  (0x27)

#define LSM303_ACC_OUT_X_L       (0x28)
#define LSM303_ACC_OUT_X_H       (0x29)
#define LSM303_ACC_OUT_Y_L       (0x2a)
#define LSM303_ACC_OUT_Y_H       (0x2b)
#define LSM303_ACC_OUT_Z_L       (0x2c)
#define LSM303_ACC_OUT_Z_H       (0x2d)

#define LSM303_ACC_FIFO_CTRL     (0x2e)
#define LSM303_ACC_SRC_REG       (0x2f)
#define LSM303_ACC_INT1_CFG      (0x30)
#define LSM303_ACC_INT1_SRC      (0x31)
#define LSM303_ACC_INT1_THS      (0x32)
#define LSM303_ACC_INT1_DURATION (0x33)
#define LSM303_ACC_INT2_CFG      (0x34)
#define LSM303_ACC_INT2_SRC      (0x35)
#define LSM303_ACC_INT2_THS      (0x36)
#define LSM303_ACC_INT2_DURATION (0x37)

#define LSM303_ACC_CLICK_CFG     (0x38)
#define LSM303_ACC_SRC           (0x39)
#define LSM303_ACC_THS           (0x3a)
#define LSM303_ACC_TIME_LIMIT    (0x3b)
#define LSM303_ACC_TIME_LATENCY  (0x3c)
#define LSM303_ACC_TIME_WINDOW   (0x3d)

/*
 * Everything related to the MAG subdevice
 */

#define LSM303_MAG_CRA           (0x00)
#define LSM303_MAG_CRB           (0x01)
#define LSM303_MAG_MR            (0x02)

#define LSM303_MAG_OUT_X_H       (0x03)
#define LSM303_MAG_OUT_X_L       (0x04)
#define LSM303_MAG_OUT_Y_H       (0x05)
#define LSM303_MAG_OUT_Y_L       (0x06)
#define LSM303_MAG_OUT_Z_H       (0x07)
#define LSM303_MAG_OUT_Z_L       (0x08)

#define LSM303_MAG_SR            (0x09)
#define LSM303_MAG_IRA           (0x0a)
#define LSM303_MAG_IRB           (0x0b)
#define LSM303_MAG_IRC           (0x0c)

#define LSM303_MAG_TEMP_OUT_H    (0x31)
#define LSM303_MAG_TEMP_OUT_L    (0x32)

void LSM303_setupSensor(void);

#endif /* LSM303_H_ */
