/*
 * This file provides a simple interface for i2c communication..
 * There are two implementations available so far:
 * 	- A nonblocking implementation, using a state-machine and interrupts.
 * 	- A blocking implementation without any interrupts
 * */

#ifndef I2CCOMMUNICATION_H_
#define I2CCOMMUNICATION_H_

#include <flawless/stdtypes.h>

/* i2c interface used for the sensor stick*/
#define SENSOR_STICK_I2C I2C2


typedef enum en_ic2Error
{
	I2C_ERROR_NONE = 0,
	I2C_ERROR_TIMEOUT = 1,
	I2C_ERROR_PEC = 2,
	I2C_ERROR_OVER_UNDERRUN = 3,
	I2C_ERROR_ACKNOWLEDGE = 4,
	I2C_ERROR_ARBITRATION_LOST = 5,
	I2C_ERROR_BUS_ERROR = 6,
	I2C_ERROR_UNEXPECTED_INTERRUPT = 7,
} i2cError_t;

typedef uint8_t i2c_slaveAddress_t;
typedef uint8_t i2c_register_t;

/* interface for a i2cCallback.  */
typedef void (*i2cCallback_t)(uint8_t *receivedData,
								uint8_t dataCnt,
								i2c_slaveAddress_t device,
								i2c_register_t subReg);

/*
 * Transfer data to and from an i2c device
 * when just reading from it tx_data must be NULL and tx_len must be 0
 * when just writing to it rx_data must be NULL and rx_len must be 0
 *
 */
bool i2c_transact(const i2c_slaveAddress_t slaveAddress,
				const uint8_t subAddr,
				const uint8_t* tx_data,
				const uint8_t tx_len,
				uint8_t* rx_data,
				const uint8_t rx_len,
				const i2cCallback_t callback);

void i2cSensorStickSetUpPins();
void i2cSensorStickForceReset();
#endif /* I2CCOMMUNICATION_H_ */
