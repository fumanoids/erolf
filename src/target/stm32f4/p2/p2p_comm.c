/*
 * p2p_comm.c
 *
 *  Created on: Nov 1, 2012
 *      Author: lutz
 */


/*
 * this is the processor to processor communication
 * this interface is numbered as logic 0
 */

#define P2P_COM_LOGIC_INTERFACE_NR 0U

#include <flawless/init/systemInitializer.h>
#include <flawless/protocol/genericFlawLessProtocol_Data.h>
#include <flawless/platform/system.h>
#include <flawless/core/msg_msgPump.h>
#include <flawless/timer/swTimer.h>

#include <framework/flawless/protocol/genericFlawLessProtocol.h>
#include <framework/flawless/core/systemMsgIDs.h>

#include <libopencm3/stm32/f4/usart.h>
#include <libopencm3/stm32/f4/rcc.h>
#include <libopencm3/stm32/f4/gpio.h>
#include <libopencm3/stm32/f4/nvic.h>
#include <libopencm3/stm32/f4/dma.h>

#include <target/stm32f4/clock.h>
#include <target/stm32f4/gpio_interrupts.h>

#include <interfaces/systemTime.h>
#include <interfaces/p2p_interface.h>

#define GEN_FIFO_USE_ATOMIC
#define GEN_FIFO_CLI_FUNCTION system_mutex_lock();
#define GEN_FIFO_SEI_FUNCTION system_mutex_unlock();
#include <flawless/misc/fifo/genericFiFo.h>

#define TX_FIFO_CNT 128U

CREATE_GENERIC_FIFO(uint8_t, p2p, TX_FIFO_CNT);

typedef enum tag_sendState
{
	SEND_STATE_IDLE,
	SEND_STATE_PAUSED
} sendState_t;

static volatile sendState_t g_sendState = SEND_STATE_IDLE;

#define P2P_USART_INTERFACE USART1


#define P2P_RX_PACKET_SGIN_PORT GPIOA
#define P2P_TX_PACKET_SGIN_PORT GPIOA

#define P2P_RX_PACKET_SGIN_PIN GPIO11
#define P2P_TX_PACKET_SGIN_PIN GPIO12


#define P2P_USART_PORT GPIOA
#define P2P_USART_TX_PIN  GPIO9
#define P2P_USART_RX_PIN  GPIO10

#define P2P_TX_DMA DMA2
#define P2P_RX_DMA DMA2

#define P2P_TX_DMA_STREAM DMA_STREAM_7
#define P2P_RX_DMA_STREAM DMA_STREAM_5

#define P2P_TX_DMA_CHANNEL 4U
#define P2P_RX_DMA_CHANNEL 4U

static p2p_fifoHandle_t g_txFifo;
static p2p_fifoHandle_t g_txFrameStopIndexesFifo;

static volatile uint8_t g_currentTransmissionCount = 0U;


#ifdef FLAWLESS_PROTOCOL_INTERFACE_STATISTICS

typedef struct tag_genericProtocolInterfaceStatistics
{
	uint16_t totalPackets;
	uint16_t checkSumErrors;
	uint16_t syncErrors;
	uint16_t sizeErrors;
} genericProtocolInterfaceStatistics_t;

typedef genericProtocolInterfaceStatistics_t genericProtocolInterfaceStatisticsArray_t[3];

extern genericProtocolInterfaceStatisticsArray_t g_interfaceStatistics;
#endif

void phySendFunction0(const flawLessTransportSymbol_t *i_data, uint16_t i_packetLen);
static void trigger_transmision(void);

static genericProtocoll_Packet_t *g_rxPacket = NULL;


static void onSendStateTimer(void)
{
	g_sendState = SEND_STATE_IDLE;
	trigger_transmision();
}

void phySendFunction0_EndOfFrameMarker()
{

	system_mutex_lock();
	{
		if (0 == g_txFifo.count)
		{
			/* this is a rare case... the fifo is empty and this is the end of a frame */
			gpio_toggle(P2P_TX_PACKET_SGIN_PORT, P2P_TX_PACKET_SGIN_PIN);
			g_sendState = SEND_STATE_PAUSED;
			swTimer_registerOnTimerUS(&onSendStateTimer, P2P_PACKET_PAUSE_INTERVAL_US, true);
			system_mutex_unlock();
		} else
		{
			uint8_t nextStopIndex = g_txFifo.end;
			system_mutex_unlock();
			while (p2p_FIFO_OKAY != p2p_put(&nextStopIndex, &g_txFrameStopIndexesFifo));
		}
	}
}

static void trigger_transmision(void)
{
	/* as long as the level is high we are receiving data
	 * so setup the dma for reception
	 */
	system_mutex_lock();
	uint8_t nextEndIndex = 0xff;
	uint8_t bytesToSendCurrently = MIN(TX_FIFO_CNT - g_txFifo.start, g_txFifo.count);

	if ((bytesToSendCurrently > 0U) &&
		(0 == g_currentTransmissionCount) &&
		(SEND_STATE_IDLE == g_sendState))
	{
		while (0 != (DMA_SCCR(P2P_TX_DMA, P2P_TX_DMA_STREAM) & DMA_CR_EN))
		{
			DMA_SCCR(P2P_TX_DMA, P2P_TX_DMA_STREAM) &= ~DMA_CR_EN;
		}

		/* check for the end of the current frame */
		{
			const uint8_t cnt = p2p_getCount(&g_txFrameStopIndexesFifo);
			if (cnt > 0U)
			{
				p2p_peek(&g_txFrameStopIndexesFifo, 0, &nextEndIndex);
				/* this is the maximum amount of data to transmit */
				if (g_txFifo.start <= nextEndIndex)
				{
					ASSERT(nextEndIndex > g_txFifo.start);
					bytesToSendCurrently = nextEndIndex - g_txFifo.start;
					if (0 == bytesToSendCurrently)
					{
						p2p_get(&g_txFrameStopIndexesFifo, &nextEndIndex); /* pop */
						gpio_toggle(P2P_TX_PACKET_SGIN_PORT, P2P_TX_PACKET_SGIN_PIN);
						g_sendState = SEND_STATE_PAUSED;
						swTimer_registerOnTimerUS(&onSendStateTimer, P2P_PACKET_PAUSE_INTERVAL_US, true);
						system_mutex_unlock();
						return;
					}
				}
			}
		}

		ASSERT(bytesToSendCurrently > 0);
		ASSERT(bytesToSendCurrently <= TX_FIFO_CNT);

		/* set txBuffer as source */
		DMA_SM0AR(P2P_TX_DMA, P2P_TX_DMA_STREAM) = (uint32_t) &(g_txFifo.data[g_txFifo.start]);
		DMA_SNDTR(P2P_TX_DMA, P2P_TX_DMA_STREAM) = bytesToSendCurrently;
		g_currentTransmissionCount = bytesToSendCurrently;

		DMA_HIFCR(P2P_TX_DMA) = (0x3d << 22);

		USART_SR(P2P_USART_INTERFACE) &= ~USART_SR_TC;
		USART_CR1(P2P_USART_INTERFACE) |= USART_CR1_TCIE;
		USART_CR3(P2P_USART_INTERFACE) |= USART_CR3_DMAT;
		/* enable dma */
		DMA_SCCR(P2P_TX_DMA, P2P_TX_DMA_STREAM)  |= DMA_CR_EN;
	}
	system_mutex_unlock();
}

/*
 * send function called from framework
 */
void phySendFunction0(const flawLessTransportSymbol_t *i_data, uint16_t i_packetLen)
{
	uint16_t i = 0U;
	for ( i = 0U; i < i_packetLen; ++i)
	{
		/* this is a busy wait for filling the fifo */
		while (p2p_FIFO_OKAY != p2p_put(&(i_data[i]), &g_txFifo))
		{
			trigger_transmision();
		}
		trigger_transmision();
	}
}

void usart1_isr(void)
{
	if (0 != (USART_SR(P2P_USART_INTERFACE) & USART_SR_TC))
	{
		system_mutex_lock();
		const uint16_t bytesTransmitted = g_currentTransmissionCount - DMA_SNDTR(P2P_TX_DMA, P2P_TX_DMA_STREAM);
		ASSERT(g_txFifo.count >= bytesTransmitted);
		g_txFifo.start = (g_txFifo.start + bytesTransmitted) % TX_FIFO_CNT;
		g_txFifo.count -= bytesTransmitted;
		g_currentTransmissionCount = 0;

		ASSERT(0 == g_currentTransmissionCount);

		USART_SR(P2P_USART_INTERFACE) &= ~USART_SR_TC;
		USART_CR1(P2P_USART_INTERFACE) &= ~USART_CR1_TCIE;
		USART_CR3(P2P_USART_INTERFACE) &= ~USART_CR3_DMAT;

		/* disable dma */
		DMA_SCCR(P2P_TX_DMA, P2P_TX_DMA_STREAM) &= ~DMA_CR_EN;

		(void)DMA_HISR(P2P_TX_DMA);
		DMA_HIFCR(P2P_TX_DMA) = (0x3d << 22);

		/* test for the frame end */
		{
			const uint8_t cnt = p2p_getCount(&g_txFrameStopIndexesFifo);
			if (cnt > 0U)
			{
				uint8_t nextEndIndex = 0U;
				p2p_peek(&g_txFrameStopIndexesFifo, 0, &nextEndIndex);
				if (g_txFifo.start == nextEndIndex)
				{
					/* this was the end of a frame */
					p2p_get(&g_txFrameStopIndexesFifo, &nextEndIndex); /* pop */
					gpio_toggle(P2P_TX_PACKET_SGIN_PORT, P2P_TX_PACKET_SGIN_PIN);
					g_sendState = SEND_STATE_PAUSED;
					swTimer_registerOnTimerUS(&onSendStateTimer, P2P_PACKET_PAUSE_INTERVAL_US, true);
				}
			}
		}

		system_mutex_unlock();
		trigger_transmision();
	}
}

void dma2_stream5_isr(void)
{
	DMA_HIFCR(P2P_RX_DMA) = (DMA_HISR(P2P_RX_DMA) & (0x3d << 6));

	if (NULL != g_rxPacket)
	{
		const uint8_t bytesReceived = FLAWLESS_PROTOCOL_MAX_PACKET_LEN - DMA_SNDTR(P2P_RX_DMA, P2P_RX_DMA_STREAM);

		if (0 != (USART_SR(P2P_USART_INTERFACE) & USART_SR_ORE))
		{
			const uint8_t dummy = USART_DR(P2P_USART_INTERFACE);
			UNUSED(dummy);
		}

		if (bytesReceived >= sizeof(g_rxPacket->subProtocolID) + sizeof(genericProtocol_Checksum_t))
		{
			g_rxPacket->interface = P2P_COM_LOGIC_INTERFACE_NR;
			g_rxPacket->payloadLen = bytesReceived - (sizeof(g_rxPacket->subProtocolID) + sizeof(genericProtocol_Checksum_t));

			/* test the checksum */
			{
				uint8_t i = 0U;
				genericProtocol_Checksum_t checksum = g_rxPacket->subProtocolID;
				for (i = 0U; i < bytesReceived - 1U; ++i)
				{
					checksum += g_rxPacket->packet[i];
				}

				if (0 == checksum)
				{
					const bool postSuccess = msgPump_postMessage(MSG_ID_GENERIC_PROTOCOL_PACKET_READY, g_rxPacket);
					ASSERT(false != postSuccess);
					if (false != postSuccess)
					{
						msgPump_getFreeBuffer(MSG_ID_GENERIC_PROTOCOL_PACKET_READY, (void*)&g_rxPacket);
					}
				} else
				{
					UNUSED(checksum); /* something dummy here... */
#ifdef FLAWLESS_PROTOCOL_INTERFACE_STATISTICS
					g_interfaceStatistics[0].checkSumErrors += 1;
#endif
				}
			}
		}
	}

	if (NULL == g_rxPacket)
	{
		msgPump_getFreeBuffer(MSG_ID_GENERIC_PROTOCOL_PACKET_READY, (void*)&g_rxPacket);

#ifdef FLAWLESS_PROTOCOL_INTERFACE_STATISTICS
		if (NULL == g_rxPacket)
		{
			g_interfaceStatistics[0].syncErrors += 1;
		}
#endif
	}

	if(NULL != g_rxPacket)
	{
		/* resetup the dma */
		/* set rxBuffer as target */
		DMA_SM0AR(P2P_RX_DMA, P2P_RX_DMA_STREAM) = (uint32_t) &(g_rxPacket->subProtocolID);
		DMA_SNDTR(P2P_RX_DMA, P2P_RX_DMA_STREAM) = FLAWLESS_PROTOCOL_MAX_PACKET_LEN;

		/* enable dma */
		DMA_SCCR(P2P_RX_DMA, P2P_RX_DMA_STREAM) |= DMA_CR_EN;
	}
}


static void onRxSigFlank(void * unused)
{
	/* this was an end of packet marker */
	/* disable the reception */
	if (0 == (DMA_SCCR(P2P_RX_DMA, P2P_RX_DMA_STREAM) & DMA_CR_EN))
	{
		/* the dma is not running, we can process that packet straight away */
		dma2_stream5_isr();
	} else
	{
		/* process the packet after the reception is complete */
		DMA_SCCR(P2P_RX_DMA, P2P_RX_DMA_STREAM) &= ~DMA_CR_EN;
	}

#ifdef FLAWLESS_PROTOCOL_INTERFACE_STATISTICS
	g_interfaceStatistics[0].totalPackets += 1;
#endif

	UNUSED(unused);
}

static void p2p_com_init(void);
MODULE_INIT_FUNCTION(p2p, 5, p2p_com_init);
static void p2p_com_init(void)
{
	p2p_init(&g_txFifo);
	p2p_init(&g_txFrameStopIndexesFifo);

	p2p_peek(&g_txFifo, 0U, NULL); /* dummy call to suppress compiler warnings */
	p2p_getCount(&g_txFifo);       /* dummy call to suppress compiler warnings */
	p2p_get(NULL, NULL);       /* dummy call to suppress compiler warnings */

	msgPump_getFreeBuffer(MSG_ID_GENERIC_PROTOCOL_PACKET_READY, (void*)&g_rxPacket);

	ASSERT(NULL != g_rxPacket);

	g_sendState = SEND_STATE_IDLE;

	/* setup USART for communication */

	RCC_APB2ENR |= RCC_APB2ENR_USART1EN;
	RCC_AHB1ENR |= RCC_AHB1ENR_IOPAEN;
	RCC_AHB1ENR |= RCC_AHB1ENR_DMA2EN;

	gpio_mode_setup(P2P_USART_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, P2P_USART_TX_PIN);
	gpio_set_output_options(P2P_USART_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, P2P_USART_TX_PIN);
	gpio_set_af(P2P_USART_PORT, GPIO_AF7, P2P_USART_TX_PIN);

	gpio_mode_setup(P2P_USART_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, P2P_USART_RX_PIN);
	gpio_set_af(P2P_USART_PORT, GPIO_AF7, P2P_USART_RX_PIN);

	gpio_mode_setup(P2P_TX_PACKET_SGIN_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, P2P_TX_PACKET_SGIN_PIN);
	gpio_set_output_options(P2P_TX_PACKET_SGIN_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, P2P_TX_PACKET_SGIN_PIN);

	gpio_registerFor_interrupt(&onRxSigFlank, P2P_RX_PACKET_SGIN_PORT, P2P_RX_PACKET_SGIN_PIN, GPIO_TRIGGER_LEVEL_FALLING | GPIO_TRIGGER_LEVEL_RISING, NULL);
	gpio_enable_interrupt(P2P_RX_PACKET_SGIN_PORT, P2P_RX_PACKET_SGIN_PIN);

	nvic_enable_irq(NVIC_USART1_IRQ);
	nvic_enable_irq(NVIC_DMA2_STREAM5_IRQ);

	USART_BRR(P2P_USART_INTERFACE) = CLOCK_APB2_CLK / P2P_USART_INTERFACE_SPEED;
	USART_CR1(P2P_USART_INTERFACE) |= USART_CR1_TE | USART_CR1_RE;
	USART_CR3(P2P_USART_INTERFACE) |= USART_CR3_DMAR | USART_CR3_DMAT;
	USART_CR1(P2P_USART_INTERFACE) |= USART_CR1_UE;


	/********* RX DMA **********/
	DMA_SCCR(P2P_RX_DMA, P2P_RX_DMA_STREAM)  = (P2P_RX_DMA_CHANNEL << DMA_CR_CHSEL_LSB) | DMA_CR_MINC | DMA_CR_TCIE;
	/* read from usart_dr */
	DMA_SPAR(P2P_RX_DMA, P2P_RX_DMA_STREAM)  = (uint32_t) &USART_DR(P2P_USART_INTERFACE);
	/* set rxBuffer as target */
	DMA_SM0AR(P2P_RX_DMA, P2P_RX_DMA_STREAM) = (uint32_t) &(g_rxPacket->subProtocolID);
	DMA_SNDTR(P2P_RX_DMA, P2P_RX_DMA_STREAM) = FLAWLESS_PROTOCOL_MAX_PACKET_LEN;
	/* enable dma */
	DMA_SCCR(P2P_RX_DMA, P2P_RX_DMA_STREAM)  |= DMA_CR_EN;



	/********* TX DMA **********/
	/* write to usart_dr */
	DMA_SPAR(P2P_TX_DMA, P2P_TX_DMA_STREAM)  = (uint32_t) &USART_DR(P2P_USART_INTERFACE);
	DMA_SCCR(P2P_TX_DMA, P2P_TX_DMA_STREAM)  = ((P2P_TX_DMA_CHANNEL << DMA_CR_CHSEL_LSB) | DMA_CR_DIR | DMA_CR_MINC);
}
