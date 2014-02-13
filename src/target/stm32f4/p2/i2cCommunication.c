#include <libopencm3/stm32/f4/rcc.h>
#include <libopencm3/stm32/f4/gpio.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/nvic.h>


#include <flawless/platform/system.h>
#include <flawless/init/systemInitializer.h>
#include <flawless/core/msg_msgPump.h>
#include <flawless/config/msgIDs.h>

#include <flawless/stdtypes.h>


#include <target/stm32f4/clock.h>

#include <interfaces/i2cCommunication.h>

#define I2C_TRANSMIT	0x0
#define I2C_RECEIVE		0x1

#define I2C_PORT GPIOB
#define I2C_PIN_SCL GPIO10
#define I2C_PIN_SDA GPIO11


typedef struct tag_i2cTransaction
{
	const uint8_t* tx_data;
	uint8_t* rx_data;
	i2cCallback_t callback;
	i2c_slaveAddress_t slaveAddress;
	uint8_t subAddr;
	uint8_t tx_len;
	uint8_t rx_len;
	uint8_t tx_sent;
	uint8_t rx_reveived;
} i2cTransaction;

typedef enum tag_i2cState
{
	I2C_STATE_IDLE,
	I2C_STATE_SC_PENDING,
	I2C_STATE_ADDR_TX_PENDING,
	I2C_STATE_SUB_REGISTER_TX_PENDING,
	I2C_STATE_TX,
	I2C_STATE_RX_SWAP, /* when going from tx to rx mode */
	I2C_STATE_ADDR_RX_PENDING,
	I2C_STATE_RX
} i2cState_t;

#define GEN_FIFO_USE_ATOMIC
#define GEN_FIFO_CLI_FUNCTION system_mutex_lock();
#define GEN_FIFO_SEI_FUNCTION system_mutex_unlock()
#include <flawless/misc/fifo/genericFiFo.h>

CREATE_GENERIC_FIFO(i2cTransaction, jobs, 16)

static jobs_fifoHandle_t g_jobs;
static i2cTransaction g_currentTransaction;
static i2cState_t g_currentState = I2C_STATE_IDLE;

static void i2cSensorStickInit();

static void triggerTransmission(void)
{
	/*
	 * if there is a job to do then do it.
	 * And just do it if there is no job going on currently
	 */

	system_mutex_lock();
	if (I2C_STATE_IDLE == g_currentState)
	{
		const jobs_ErrorType_t error = jobs_get(&g_jobs, &g_currentTransaction);
		if (jobs_FIFO_OKAY == error)
		{
			/*
			 * go into SC_PENDING state
			 */
			g_currentState = I2C_STATE_SC_PENDING;
			I2C_CR2(SENSOR_STICK_I2C) |= I2C_CR2_ITERREN | I2C_CR2_ITEVTEN;
			I2C_CR1(SENSOR_STICK_I2C) |= I2C_CR1_START;
		} else
		{
			/* the queue is empty */
		}
	} else
	{
		/* we cannot do anything since we are handling a job allready */
	}
	system_mutex_unlock();
}


bool i2c_transact(const i2c_slaveAddress_t slaveAddress,
				const uint8_t subAddr,
				const uint8_t* tx_data,
				const uint8_t tx_len,
				uint8_t* rx_data,
				const uint8_t rx_len,
				const i2cCallback_t callback)
{
	bool success = FALSE;

	i2cTransaction transaction;
	transaction.slaveAddress = slaveAddress;
	transaction.subAddr      = subAddr;
	transaction.tx_data      = tx_data;
	transaction.tx_len       = tx_len;
	transaction.rx_data      = rx_data;
	transaction.rx_len       = rx_len;
	transaction.callback     = callback;
	transaction.rx_reveived  = 0;
	transaction.tx_sent      = 0;

	{
		jobs_ErrorType_t error = jobs_put(&transaction, &g_jobs);
		if (jobs_FIFO_OKAY == error)
		{
			success = TRUE;
		} else
		{
			success = FALSE;
		}
	}
	triggerTransmission();
	return success;
}

/* initialize gpio-pins for the sensor stick*/
void i2cSensorStickSetUpPins()
{
	RCC_AHB1ENR |= RCC_AHB1ENR_IOPBEN;

	/* setup ports */
	gpio_mode_setup(I2C_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, I2C_PIN_SCL);
	gpio_mode_setup(I2C_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, I2C_PIN_SDA);

	gpio_set_output_options(I2C_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_100MHZ, I2C_PIN_SCL);
	gpio_set_output_options(I2C_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_100MHZ, I2C_PIN_SDA);

	gpio_set_af(I2C_PORT, GPIO_AF4, I2C_PIN_SCL);
	gpio_set_af(I2C_PORT, GPIO_AF4, I2C_PIN_SDA);
}

/* This method can be used if one or more i2c-slaves are crashed and therefore hold the sda line on low.
 * It will force both, the sda and scl line to low for a short time, which will reset most i2c-clients.
 * */
void i2cSensorStickForceReset()
{
	/* this performs a propper reset! */
	volatile systemTime_t timeout;

	I2C_CR1(SENSOR_STICK_I2C) |= I2C_CR1_SWRST;
	I2C_CR1(SENSOR_STICK_I2C) &= ~I2C_CR1_SWRST;


	gpio_mode_setup(I2C_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, I2C_PIN_SCL);
	gpio_mode_setup(I2C_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, I2C_PIN_SDA);

	gpio_set_output_options(I2C_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, I2C_PIN_SCL);
	gpio_set_output_options(I2C_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, I2C_PIN_SDA);

	gpio_clear(I2C_PORT, I2C_PIN_SCL);
	gpio_clear(I2C_PORT, I2C_PIN_SDA);

	timeout = getSystemTimeUS();
	while (timeout + 1000 > getSystemTimeUS());


	i2cSensorStickSetUpPins();
	/* disable periphal*/
	I2C_CR1(SENSOR_STICK_I2C) &= ~I2C_CR1_PE;
	I2C_CR2(SENSOR_STICK_I2C) &= ~(I2C_CR2_ITERREN | I2C_CR2_ITEVTEN);

	i2cSensorStickInit();

	g_currentState = I2C_STATE_IDLE;
}

/* initialize i2c for the sensor stick*/

static void i2cSensorStickInit()
{
	// enable i2c clock
	RCC_APB1ENR |= RCC_APB1ENR_I2C2EN;

	/* disable peripheral */
	I2C_CR1(SENSOR_STICK_I2C) &= ~I2C_CR1_PE;

	// set APB input frequency
	I2C_CR2(SENSOR_STICK_I2C) |= (CLOCK_APB1_CLK / 1000000LLU);


	I2C_CCR(SENSOR_STICK_I2C) |= I2C_CCR_FS;
	I2C_CCR(SENSOR_STICK_I2C) |= I2C_CCR_DUTY;
	I2C_CCR(SENSOR_STICK_I2C) |= 3; /* for about 400kHz Baud */


	/*15 = 36 * 400 / 1000 + 1 (according to stm32 standard periphal)*/
	I2C_TRISE(SENSOR_STICK_I2C) |= 0x3;

	/* enable event and error interrupt. */
	nvic_enable_irq(NVIC_I2C2_EV_IRQ);
	nvic_set_priority(NVIC_I2C2_EV_IRQ, 1);
	nvic_enable_irq(NVIC_I2C2_ER_IRQ);

	/* enable periphal*/
	I2C_CR1(SENSOR_STICK_I2C) |= I2C_CR1_PE;
}

/*
 * I2C event ISR
 */
void i2c2_ev_isr()
{
	int statReg1 = I2C_SR1(SENSOR_STICK_I2C);
	int statReg2 = I2C_SR2(SENSOR_STICK_I2C);
	UNUSED(statReg1);
	UNUSED(statReg2);
	switch (g_currentState)
	{
		case I2C_STATE_SC_PENDING:
			if (0 != (statReg1 & I2C_SR1_SB))
			{
				/* start transmitting the remote address for a write */
				g_currentState = I2C_STATE_ADDR_TX_PENDING;
				I2C_DR(SENSOR_STICK_I2C) = (g_currentTransaction.slaveAddress & 0xfe); /* last bit cleared for a write */
			}
			else
			{
				/* error handling... retry creating start condition */
				g_currentState = I2C_STATE_IDLE;
				I2C_CR1(SENSOR_STICK_I2C) |= I2C_CR1_STOP;
				triggerTransmission();
			}
			break;

		case I2C_STATE_ADDR_TX_PENDING:
			if (0 != (statReg1 & I2C_SR1_ADDR))
			{
				/* transmit the sub address */
				if (0 != (statReg1 & I2C_SR1_TxE))
				{
					uint8_t subAddr = g_currentTransaction.subAddr;
					if (((NULL != g_currentTransaction.rx_data) &&
						(1 < g_currentTransaction.rx_len)) ||
						((NULL != g_currentTransaction.tx_data) &&
						(1 < g_currentTransaction.tx_len)))
					{
						/* if this is a multi byte access we have to assert the MSB of subAddr */
						subAddr |= (1 << 7);
					}
					I2C_DR(SENSOR_STICK_I2C) = subAddr;
					g_currentState = I2C_STATE_TX;
				}
			}
			else
			{
				/* this transaction wont go anywhere... there is no slave with that address */
				if (0 != (statReg1 & I2C_SR1_AF) || 0 != (statReg1 & I2C_SR1_SB))
				{
					/* the slave did not respond ... AF = Acknowledge Failure */
					/* go on transmitting the next job */
					g_currentState = I2C_STATE_IDLE;
					I2C_CR1(SENSOR_STICK_I2C) |= I2C_CR1_STOP;
					I2C_SR1(SENSOR_STICK_I2C) &= ~I2C_SR1_AF;
					triggerTransmission();
				}
			}
			break;

		case I2C_STATE_TX:
			if ((NULL != g_currentTransaction.tx_data)
				&& (0 != g_currentTransaction.tx_len))
			{
				if (g_currentTransaction.tx_len > g_currentTransaction.tx_sent)
				{
					/* we go on transmitting data */
					if (0 != (statReg1 & I2C_SR1_TxE))
					{
						I2C_DR(SENSOR_STICK_I2C) = g_currentTransaction.tx_data[g_currentTransaction.tx_sent];
						++g_currentTransaction.tx_sent;
						g_currentState = I2C_STATE_TX;

						/* if this was the last byte to send AND there is no receive afterwards
						 * let go of the bus
						 */
						if ((g_currentTransaction.tx_sent == g_currentTransaction.tx_len)
							&& (NULL == g_currentTransaction.rx_data)
							&& (0 == g_currentTransaction.rx_len))
						{
							I2C_CR1(SENSOR_STICK_I2C) |= I2C_CR1_STOP;
						}
					}
				} else
				{
					/* enter receive mode if we can receive stuff */
					if ((NULL != g_currentTransaction.rx_data)
						&& (0 != g_currentTransaction.rx_len))
					{
						g_currentState = I2C_STATE_RX_SWAP;
						I2C_CR1(SENSOR_STICK_I2C) |= I2C_CR1_START;
					} else
					{
						/* the transaction is done
						 * go for the next job
						 * */
						g_currentState = I2C_STATE_IDLE;
						triggerTransmission();
					}
				}
			} else
			{
				/* enter receive mode if we can receive stuff */
				if ((NULL != g_currentTransaction.rx_data)
					&& (0 != g_currentTransaction.rx_len))
				{
					g_currentState = I2C_STATE_RX_SWAP;
					I2C_CR1(SENSOR_STICK_I2C) |= I2C_CR1_START;
				} else
				{
					/* the transaction is done */
					g_currentState = I2C_STATE_IDLE;
					triggerTransmission();
				}
			}
			break;

		case I2C_STATE_RX_SWAP:
			{
				if (0 != (statReg1 & I2C_SR1_SB))
				{
					/* start transmitting the remote address for a read */
					I2C_DR(SENSOR_STICK_I2C) = (g_currentTransaction.slaveAddress | 0x01); /* last bit set for a read */
					g_currentState = I2C_STATE_ADDR_RX_PENDING;
				}
			}
			break;

		case I2C_STATE_ADDR_RX_PENDING:
			if (0 != (statReg1 & I2C_SR1_ADDR))
			{
				/* we can receive data from slave */
				g_currentState = I2C_STATE_RX;
				if (g_currentTransaction.rx_len > 1)
				{
					I2C_CR1(SENSOR_STICK_I2C) |= I2C_CR1_ACK;

					/* read from I2CR2 to clear the ADDR bit */
					(void)I2C_SR2(SENSOR_STICK_I2C);
				} else
				{
					/* if we just want to receive one byte then the ACK bit has to be cleared */
					I2C_CR1(SENSOR_STICK_I2C) &= ~I2C_CR1_ACK;

					/* read from I2CR2 to clear the ADDR bit */
					(void)I2C_SR2(SENSOR_STICK_I2C);

					I2C_CR1(SENSOR_STICK_I2C) |= I2C_CR1_STOP;
				}
			}
			break;

		case I2C_STATE_RX:
			if ((I2C_SR1_BTF | I2C_SR1_RxNE) == (statReg1 & (I2C_SR1_BTF | I2C_SR1_RxNE)))
			{
				const uint8_t pendingBytes = g_currentTransaction.rx_len - g_currentTransaction.rx_reveived;

				const uint8_t received = I2C_DR(SENSOR_STICK_I2C);
				g_currentTransaction.rx_data[g_currentTransaction.rx_reveived] = received;
				++g_currentTransaction.rx_reveived;

				if (1U == pendingBytes)
				{
					/*
					 * disable the ack bit
					 */
					I2C_CR1(SENSOR_STICK_I2C) &= ~I2C_CR1_ACK;
					I2C_CR1(SENSOR_STICK_I2C) |= I2C_CR1_STOP;
				}

				if (g_currentTransaction.rx_reveived == g_currentTransaction.rx_len)
				{
					/* call the callback */
					if (NULL != g_currentTransaction.callback)
					{
						(void)(g_currentTransaction.callback)(g_currentTransaction.rx_data,
																g_currentTransaction.rx_reveived,
																g_currentTransaction.slaveAddress,
																g_currentTransaction.subAddr);
					}
					/* go into idle state */
					g_currentState = I2C_STATE_IDLE;
					triggerTransmission();
				}
			}
			break;

		default:
			/* clear everything that generates interrups... */
			I2C_CR2(SENSOR_STICK_I2C) &= ~I2C_CR2_ITEVTEN;
//			i2cSensorStickForceReset();
			(void)I2C_SR1(SENSOR_STICK_I2C);
			(void)I2C_SR2(SENSOR_STICK_I2C);

			I2C_CR1(SENSOR_STICK_I2C) |= I2C_CR1_SWRST;
			I2C_CR1(SENSOR_STICK_I2C) &= ~I2C_CR1_SWRST;
			i2cSensorStickInit();
			break;
	}

	/* flush the interface */
	if (I2C_STATE_RX != g_currentState)
	{
		if (0 != (I2C_SR1(SENSOR_STICK_I2C) & I2C_SR1_RxNE))
		{
			(void)I2C_DR(SENSOR_STICK_I2C);
		}
	}

	triggerTransmission();
}

/*
 * I2C error ISR
 */
void i2c2_er_isr(void)
{
	/* start over again */
	if (0 != (I2C_SR1(SENSOR_STICK_I2C) & (I2C_SR1_AF)))
	{
		g_currentState = I2C_STATE_IDLE;
	} else if (0 != (I2C_SR1(SENSOR_STICK_I2C) & (I2C_SR1_BERR | I2C_SR1_ARLO)))
	{
		i2cSensorStickForceReset();
	}
	I2C_SR1(SENSOR_STICK_I2C) &= ~(I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_AF);
	triggerTransmission();
}


static void i2c_init(void);
MODULE_INIT_FUNCTION(i2c, 5, i2c_init)
static void i2c_init(void)
{
	i2cSensorStickSetUpPins();
	i2cSensorStickInit();

	jobs_getCount(NULL); /* call that function to preven compiler complains */
	jobs_peek(NULL, 0, NULL);

	g_currentState = I2C_STATE_IDLE;
	jobs_init(&g_jobs);
}

