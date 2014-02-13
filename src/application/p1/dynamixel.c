/*
 * dynamixel.c
 *
 *  Created on: Dec 1, 2012
 *      Author: lutz
 */


#include <flawless/init/systemInitializer.h>
#include <flawless/stdtypes.h>
#include <flawless/core/msg_msgPump.h>
#include <flawless/config/msgIDs.h>


#include <interfaces/dynamixel_config.h>
#include <interfaces/dynamixel.h>

#include <flawless/timer/swTimer.h>

#include "target/stm32f4/p1/dynamixel_phy.h"

#include <flawless/platform/system.h>

#define DYNAMIXEL_MAX_PACKET_LEN 128U

typedef uint8_t dynamixelPacket_t[DYNAMIXEL_MAX_PACKET_LEN];

#define GEN_FIFO_USE_ATOMIC 1
#define GEN_FIFO_CLI_FUNCTION system_mutex_lock();
#define GEN_FIFO_SEI_FUNCTION system_mutex_unlock();
#include <flawless/misc/fifo/genericFiFo.h>

typedef struct tag_dynamixelTransaction
{
	dynamixel_Callback_t callback;
	dynamixelPacket_t tx_data;
	uint8_t tx_len;
	bool needsAnswer;
	uint8_t rx_len;
} dynamixelTransaction;

CREATE_GENERIC_FIFO(dynamixelTransaction, jobs, 32);
static jobs_fifoHandle_t g_LowPrioJobs[DYNAMIXEL_PORT_CNT];
static jobs_fifoHandle_t g_HighPrioJobs[DYNAMIXEL_PORT_CNT];

#define DYNAMIXEL_STATUS_PACKET_MIN_LENGTH 6U


MSG_PUMP_DECLARE_MESSAGE_BUFFER_POOL(incommingDynamixelDataBP, dynamixelStatusPacket_t, 10, MSG_ID_INCOMMING_DYNAMIXEL_PACKET)


static dynamixelTransaction g_currentTransactions[DYNAMIXEL_PORT_CNT];

static dynamixelPacket_t g_incommingPackets[DYNAMIXEL_PORT_CNT];

typedef enum tagReceiveState
{
	RECEIVE_STATE_NOTHING,
	RECEIVE_STATE_ID,
	RECEIVE_STATE_LENGTH,
	RECEIVE_STATE_ERROR,
	RECEIVE_STATE_PARAMETERS,
	RECEIVE_STATE_CHECKSUM
} receiveState_t;

typedef enum phyState
{
	DYNAMIXEL_PHY_STATE_IDLE = 0U,
	DYNAMIXEL_PHY_STATE_RX = 1U,
	DYNAMIXEL_PHY_STATE_TX = 2U
} phyState_t;

typedef struct tag_receiveMode
{
	receiveState_t receiveState;
	dynamixelPacket_t payload;
	uint8_t payloadBytesReceived;
	uint8_t payloadLen;
	uint8_t headerBytesReceived;
	uint8_t checksum;
} receiveMode_t;




static receiveMode_t g_receiveModes[DYNAMIXEL_PORT_CNT];

static volatile phyState_t g_phyState[DYNAMIXEL_PORT_CNT];

static void triggerTransmission(dynamixel_port_t port);
static void triggerTransmission(dynamixel_port_t port)
{
	system_mutex_lock();
	if ((DYNAMIXEL_PORT_CNT > port) &&
		(DYNAMIXEL_PHY_STATE_IDLE == g_phyState[port]))
	{
		const jobs_ErrorType_t highError = jobs_get(&(g_HighPrioJobs[port]), &(g_currentTransactions[port]));
		if (jobs_FIFO_OKAY == highError)
		{
			g_phyState[port] = DYNAMIXEL_PHY_STATE_TX;
			g_receiveModes[port].receiveState = RECEIVE_STATE_NOTHING;
			dynamixel_phy_send(port, (uint8_t*)&(g_currentTransactions[port].tx_data), g_currentTransactions[port].tx_len);
		} else
		{
			/* do the next low prio transaction */
			const jobs_ErrorType_t lowError = jobs_get(&(g_LowPrioJobs[port]), &(g_currentTransactions[port]));
			if (jobs_FIFO_OKAY == lowError)
			{
				g_phyState[port] = DYNAMIXEL_PHY_STATE_TX;
				g_receiveModes[port].receiveState = RECEIVE_STATE_NOTHING;
				dynamixel_phy_send(port, (uint8_t*)&(g_currentTransactions[port].tx_data), g_currentTransactions[port].tx_len);
			}
		}
	} else if (DYNAMIXEL_PORT_ALL == port)
	{
		for (uint8_t i = DYNAMIXEL_PORT_1; i < DYNAMIXEL_PORT_CNT; ++i)
		{
			if (DYNAMIXEL_PHY_STATE_IDLE == g_phyState[i])
			{
				const jobs_ErrorType_t highError = jobs_get(&(g_HighPrioJobs[i]), &(g_currentTransactions[i]));
				if (jobs_FIFO_OKAY == highError)
				{
					g_phyState[i] = DYNAMIXEL_PHY_STATE_TX;
					g_receiveModes[i].receiveState = RECEIVE_STATE_NOTHING;
					dynamixel_phy_send(i, (uint8_t*)&(g_currentTransactions[i].tx_data), g_currentTransactions[i].tx_len);
				} else
				{
					/* do the next low prio transaction */
					const jobs_ErrorType_t lowError = jobs_get(&(g_LowPrioJobs[i]), &(g_currentTransactions[i]));
					if (jobs_FIFO_OKAY == lowError)
					{
						g_phyState[i] = DYNAMIXEL_PHY_STATE_TX;
						g_receiveModes[i].receiveState = RECEIVE_STATE_NOTHING;
						dynamixel_phy_send(i, (uint8_t*)&(g_currentTransactions[i].tx_data), g_currentTransactions[i].tx_len);
					}
				}
			}
		}
	}
	system_mutex_unlock();
}


static void timeoutInterface1(void);
static void timeoutInterface1(void)
{
	/* a timeout happened! go for the next job */
	const dynamixel_port_t port = DYNAMIXEL_PORT_1;

	if (NULL != g_currentTransactions[port].callback)
	{
		(void)(g_currentTransactions[port].callback)(false,
													NULL,
													0,
													0,
													g_currentTransactions[port].tx_data[2],
													g_currentTransactions[port].tx_data[4]);
	}

	system_mutex_lock();
	dynamixel_phy_resetInterface(port);
	g_phyState[port] = DYNAMIXEL_PHY_STATE_IDLE;
	triggerTransmission(port);
	system_mutex_unlock();
}

static void timeoutInterface2(void);
static void timeoutInterface2(void)
{
	/* a timeout happened! go for the next job */
	const dynamixel_port_t port = DYNAMIXEL_PORT_2;

	if (NULL != g_currentTransactions[port].callback)
	{
		(void)(g_currentTransactions[port].callback)(false,
													NULL,
													0,
													0,
													g_currentTransactions[port].tx_data[2],
													g_currentTransactions[port].tx_data[4]);
	}

	system_mutex_lock();
	dynamixel_phy_resetInterface(port);
	g_phyState[port] = DYNAMIXEL_PHY_STATE_IDLE;
	triggerTransmission(port);
	system_mutex_unlock();
}

static void timeoutInterface3(void);
static void timeoutInterface3(void)
{
	/* a timeout happened! go for the next job */
	const dynamixel_port_t port = DYNAMIXEL_PORT_3;

	if (NULL != g_currentTransactions[port].callback)
	{
		(void)(g_currentTransactions[port].callback)(false,
													NULL,
													0,
													0,
													g_currentTransactions[port].tx_data[2],
													g_currentTransactions[port].tx_data[4]);
	}

	system_mutex_lock();
	dynamixel_phy_resetInterface(port);
	g_phyState[port] = DYNAMIXEL_PHY_STATE_IDLE;
	triggerTransmission(port);
	system_mutex_unlock();
}

static bool addJob(const dynamixel_Callback_t callback,
					dynamixelPacket_t tx_data,
					uint8_t tx_len,
					dynamixel_port_t port,
					bool needsAnswer,
					uint8_t answerLength,
					transactionPriority_t priority);
static bool addJob(const dynamixel_Callback_t callback,
					dynamixelPacket_t tx_data,
					uint8_t tx_len,
					dynamixel_port_t port,
					bool needsAnswer,
					uint8_t answerLength,
					transactionPriority_t priority)
{
	bool ret = false;

	if ((DYNAMIXEL_PORT_CNT > port) ||
		(DYNAMIXEL_PORT_ALL == port))
	{
		dynamixelTransaction transaction;
		transaction.callback = callback;
		transaction.needsAnswer = needsAnswer;
		transaction.rx_len =answerLength;
		memcpy(&(transaction.tx_data), tx_data, tx_len);
		transaction.tx_len = tx_len;
		if (DYNAMIXEL_PORT_ALL == port)
		{
			uint8_t i = 0U;
			ret = true;
			for (i = 0U; i < DYNAMIXEL_PORT_CNT; ++i)
			{
				jobs_ErrorType_t error = jobs_FIFO_OVERRUN;
				if (DYNAMIXEL_TRANSACTION_PRIORITY_LOW == priority)
				{
					error = jobs_put(&transaction, &(g_LowPrioJobs[i]));
				} else
				{
					error = jobs_put(&transaction, &(g_HighPrioJobs[i]));
				}
				if (jobs_FIFO_OKAY != error)
				{
					ret = false;
				}
			}
		} else
		{
			jobs_ErrorType_t error = jobs_FIFO_OVERRUN;
			if (DYNAMIXEL_TRANSACTION_PRIORITY_LOW == priority)
			{
				error = jobs_put(&transaction, &(g_LowPrioJobs[port]));
			} else
			{
				error = jobs_put(&transaction, &(g_HighPrioJobs[port]));
			}
			if (jobs_FIFO_OKAY == error)
			{
				ret = true;
			}
		}
	}

	triggerTransmission(port);

	return ret;
}

static void receptionComplete(dynamixel_port_t port, const uint8_t *rxData, uint8_t len);
static void receptionComplete(dynamixel_port_t port, const uint8_t *rxData, uint8_t len)
{
	system_mutex_lock();
	if (((DYNAMIXEL_PORT_CNT > port) ||
		(DYNAMIXEL_PORT_ALL == port)) &&
		(DYNAMIXEL_PHY_STATE_RX == g_phyState[port]))
	{
		switch (port)
		{
			case DYNAMIXEL_PORT_1:
				swTimer_unRegisterFromTimer(&timeoutInterface1);
				break;
			case DYNAMIXEL_PORT_2:
				swTimer_unRegisterFromTimer(&timeoutInterface2);
				break;
			case DYNAMIXEL_PORT_3:
				swTimer_unRegisterFromTimer(&timeoutInterface3);
				break;
			default:
				break;
		}

		g_phyState[port] = DYNAMIXEL_PHY_STATE_IDLE;

		system_mutex_unlock();

		if ((NULL != g_currentTransactions[port].callback) &&
			(len >= 5))
		{
			(void)(g_currentTransactions[port].callback)(TRUE,
														&(rxData[5]),
														rxData[3] - 2U,
														rxData[4],
														rxData[2],
														g_currentTransactions[port].tx_data[4]);
		}
	} else
	{
		system_mutex_unlock();
	}
}

static void receiveCompleteCallback(dynamixel_port_t port, uint8_t *receivedData, uint8_t len);
static void receiveCompleteCallback(dynamixel_port_t port, uint8_t *receivedData, uint8_t len)
{
	if (DYNAMIXEL_PORT_CNT > port)
	{
		const uint16_t *header = (const uint16_t*)receivedData;

		if(0xffff == (*header))
		{

			/* this is a valid packet start */
			uint8_t checksum = 0U;
			uint8_t i = 0U;
			for (i = sizeof(*header); i < len; ++i)
			{
				checksum += receivedData[i];
			}
			if ((0xff == checksum) &&
				(receivedData[3] == (len - 4U)))
			{
				receptionComplete(port, receivedData, len);

				/* this IS a valid packet! */
				dynamixelStatusPacket_t *incommingPacket = NULL;
				const bool aquireSuccess = msgPump_getFreeBuffer(MSG_ID_INCOMMING_DYNAMIXEL_PACKET, (void*)&incommingPacket);
				if (false != aquireSuccess)
				{

					incommingPacket->motorID = receivedData[2];
					incommingPacket->payloadLen = receivedData[3] - 2U;
					incommingPacket->error = receivedData[4];
					memcpy(&(incommingPacket->payload), &(receivedData[5]), incommingPacket->payloadLen);
					msgPump_postMessage(MSG_ID_INCOMMING_DYNAMIXEL_PACKET, incommingPacket);
				}
			}
		}else
		{
			uint8_t checksum = 0U;
			(void)checksum;
		}
		triggerTransmission(port);
	}
}

static void transmittCompleteCallback(dynamixel_port_t port);
static void transmittCompleteCallback(dynamixel_port_t port)
{
	if (DYNAMIXEL_PORT_CNT > port)
	{
		if (FALSE != g_currentTransactions[port].needsAnswer)
		{
			system_mutex_lock();
			/* setup for reception of an answer */
			g_phyState[port] = DYNAMIXEL_PHY_STATE_RX;
			dynamixel_phy_receive(port, (uint8_t*)&(g_incommingPackets[port]),g_currentTransactions[port].rx_len);

			switch (port)
			{
				case DYNAMIXEL_PORT_1:
					swTimer_registerOnTimer(&timeoutInterface1, DYNAMIXEL_MAX_RESPONSE_DELAY_MS, true);
					break;
				case DYNAMIXEL_PORT_2:
					swTimer_registerOnTimer(&timeoutInterface2, DYNAMIXEL_MAX_RESPONSE_DELAY_MS, true);
					break;
				case DYNAMIXEL_PORT_3:
					swTimer_registerOnTimer(&timeoutInterface3, DYNAMIXEL_MAX_RESPONSE_DELAY_MS, true);
					break;
				default:
					break;
			}
			system_mutex_unlock();

		} else
		{
			/* send the next packet straight away */
			g_phyState[port] = DYNAMIXEL_PHY_STATE_IDLE;
			triggerTransmission(port);
		}
	}
}

bool dynamixel_read(dynamixel_motorID_t addr,
					dynamixel_register_t reg,
					uint8_t cnt,
					dynamixel_port_t port,
					dynamixel_Callback_t callback,
					transactionPriority_t priority)
{
	bool ret = false;

	if ((DYNAMIXEL_PORT_CNT > port) ||
		(DYNAMIXEL_PORT_ALL == port))
	{
		uint8_t checksum = 0U;

		dynamixelPacket_t packet;

		packet[0] = 0xff;
		packet[1] = 0xff;
		packet[2] = addr;
		packet[3] = 4; /* length */
		packet[4] = DYNAMIXEL_INSTRUCTION_READ;
		packet[5] = reg;
		packet[6] = cnt;

		checksum = addr + 4 + DYNAMIXEL_INSTRUCTION_READ + reg + cnt;
		packet[7] = ~(checksum);

		ret = addJob(callback, packet, 8, port, true, DYNAMIXEL_STATUS_PACKET_MIN_LENGTH + cnt, priority);
	}
	return ret;
}

bool dynamixel_write(dynamixel_motorID_t addr,
					dynamixel_register_t reg,
					const uint8_t *data,
					uint8_t cnt,
					dynamixel_port_t port,
					dynamixel_Callback_t callback,
					bool expectsAnswer,
					transactionPriority_t priority)
{
	bool ret = false;

	if ((DYNAMIXEL_REGISTER_BAUD_RATE < reg) &&
		((DYNAMIXEL_PORT_CNT > port) ||
		(DYNAMIXEL_PORT_ALL == port)))
	{
		uint8_t i = 0U;
		uint8_t checksum = 0U;

		dynamixelPacket_t packet;

		packet[0] = 0xff;
		packet[1] = 0xff;
		packet[2] = addr;
		packet[3] = cnt + 3; /* length */
		packet[4] = DYNAMIXEL_INSTRUCTION_WRITE;
		packet[5] = reg;

		checksum += addr + (cnt + 3) + DYNAMIXEL_INSTRUCTION_WRITE + reg;

		for (i = 0; i < cnt; ++i)
		{
			packet[6 + i] = data[i];
			checksum += data[i];
		}

		packet[6 + i] = ~(checksum);

		ret = addJob(callback, packet, (i + 7), port, expectsAnswer, DYNAMIXEL_STATUS_PACKET_MIN_LENGTH, priority);
	}

	return ret;
}

bool dynamixel_ping(dynamixel_motorID_t addr,
					dynamixel_Callback_t callback,
					dynamixel_port_t port,
					transactionPriority_t priority)
{
	bool ret = false;

	if ((DYNAMIXEL_PORT_CNT > port) ||
		(DYNAMIXEL_PORT_ALL == port))
	{
		dynamixelPacket_t packet;
		packet[0] = 0xff;
		packet[1] = 0xff;
		packet[2] = addr;
		packet[3] = 2; /* length */
		packet[4] = DYNAMIXEL_INSTRUCTION_PING;
		packet[5] = ~((addr + 2 + DYNAMIXEL_INSTRUCTION_PING) & 0xff);

		ret = addJob(callback, packet, 6, port, true, DYNAMIXEL_STATUS_PACKET_MIN_LENGTH, priority);
	}

	return ret;
}

bool dynamixel_reg_write(dynamixel_motorID_t addr,
					dynamixel_register_t reg,
					const uint8_t *data,
					uint8_t cnt,
					dynamixel_port_t port,
					dynamixel_Callback_t callback,
					bool expectsAnswer,
					transactionPriority_t priority)
{
	bool ret = false;

	if ((DYNAMIXEL_REGISTER_BAUD_RATE < reg) &&
		((DYNAMIXEL_PORT_CNT > port) ||
		(DYNAMIXEL_PORT_ALL == port)))
	{
		uint8_t i = 0U;
		uint8_t checksum = 0U;

		dynamixelPacket_t packet;

		packet[0] = 0xff;
		packet[1] = 0xff;
		packet[2] = addr;
		packet[3] = cnt + 3; /* length */
		packet[4] = DYNAMIXEL_INSTRUCTION_REG_WRITE;
		packet[5] = reg;

		checksum += addr + (cnt + 3) + DYNAMIXEL_INSTRUCTION_REG_WRITE + reg;

		for (i = 0; i < cnt; ++i)
		{
			packet[6 + i] = data[i];
			checksum += data[i];
		}

		packet[6 + i] = ~(checksum);

		ret = addJob(callback, packet, (i + 7), port, expectsAnswer, DYNAMIXEL_STATUS_PACKET_MIN_LENGTH, priority);
	}
	return ret;
}

bool dynamixel_action(dynamixel_motorID_t addr,
					dynamixel_port_t port,
					transactionPriority_t priority)
{
	bool ret = false;

	if ((DYNAMIXEL_PORT_CNT > port) ||
		(DYNAMIXEL_PORT_ALL == port))
	{
		dynamixelPacket_t packet;

		packet[0] = 0xff;
		packet[1] = 0xff;
		packet[2] = addr;
		packet[3] = 2; /* length */
		packet[4] = DYNAMIXEL_INSTRUCTION_ACTION;
		packet[5] = ~(addr + 2 + DYNAMIXEL_INSTRUCTION_ACTION);

		ret = addJob(NULL, packet, 6, port, false, 0, priority);
	}
	return ret;
}

bool dynamixel_reset(dynamixel_motorID_t addr,
					dynamixel_port_t port,
					transactionPriority_t priority)
{
	bool ret = false;

	return ret;
	/* this should not be executed anyway*/

	if ((DYNAMIXEL_PORT_CNT > port) ||
		(DYNAMIXEL_PORT_ALL == port))
	{
		dynamixelPacket_t packet;

		packet[0] = 0xff;
		packet[1] = 0xff;
		packet[2] = addr;
		packet[3] = 2; /* length */
		packet[4] = DYNAMIXEL_INSTRUCTION_RESET;
		packet[5] = ~(addr + 2 + DYNAMIXEL_INSTRUCTION_RESET);

		ret = addJob(NULL, packet, 6, port, false, 0, priority);
	}
	return ret;
}

bool dynamixel_sync_write(const dynamixel_motorID_t *addr,
						uint8_t addrCnt,
						dynamixel_register_t reg,
						const uint8_t **data,
						uint8_t dataCnt,
						dynamixel_port_t port,
						transactionPriority_t priority)
{
	bool ret = false;

	if ((DYNAMIXEL_REGISTER_BAUD_RATE < reg) &&
		((DYNAMIXEL_PORT_CNT > port) ||
		(DYNAMIXEL_PORT_ALL == port)))
	{
		dynamixelPacket_t packet;

		const uint8_t length = (dataCnt + 1) * addrCnt + 4;
		uint8_t checksum = DYNAMIXEL_BROADCAST_ID + length + DYNAMIXEL_INSTRUCTION_SYNC_WRITE + reg+ dataCnt;
		uint8_t *dataPtr = &(packet[7]);
		uint8_t i = 0U;

		packet[0] = 0xff;
		packet[1] = 0xff;
		packet[2] = DYNAMIXEL_BROADCAST_ID;
		packet[3] = length; /* length */
		packet[4] = DYNAMIXEL_INSTRUCTION_SYNC_WRITE;
		packet[5] = reg;
		packet[6] = dataCnt;

		for (i = 0U; i < addrCnt; ++i)
		{
			uint8_t j = 0U;
			const uint8_t *params = data[i];

			*dataPtr = addr[i];
			checksum += *dataPtr;
			++dataPtr;
			for (j = 0U; j < dataCnt; ++j)
			{
				*dataPtr = params[j];
				checksum += *dataPtr;
				++dataPtr;
			}
		}

		packet[length + 3] = ~(checksum);

		ret = addJob(NULL, packet, (length + 4), port, false, 0, priority);
	}
	return ret;
}

static void dynamixelInitFunction(void);
MODULE_INIT_FUNCTION(dynamixel, 6, dynamixelInitFunction)
static void dynamixelInitFunction(void)
{
	uint8_t i = 0U;

	jobs_peek(NULL, 0U, NULL); /* dummy call to suppress compiler warnings */
	jobs_getCount(NULL); /* dummy call to suppress compiler warnings */

	dynamixel_phy_setRxCompleteCallback(&receiveCompleteCallback);
	dynamixel_phy_setTXCCallback(&transmittCompleteCallback);

	for (i = 0U; i < DYNAMIXEL_PORT_CNT; ++i)
	{
		jobs_init(&(g_LowPrioJobs[i]));
		jobs_init(&g_HighPrioJobs[i]);
		g_phyState[i] = DYNAMIXEL_PHY_STATE_IDLE;
		g_receiveModes[i].receiveState = RECEIVE_STATE_NOTHING;
		g_receiveModes[i].payloadBytesReceived = 0U;
		g_receiveModes[i].headerBytesReceived = 0U;
		g_receiveModes[i].payloadLen = 0U;
		memset(&(g_receiveModes[i].payload), 0, sizeof(g_receiveModes[i].payload));
	}

}

