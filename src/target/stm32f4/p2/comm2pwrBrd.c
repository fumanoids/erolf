/*
 * data_comm.c
 *
 *  Created on: Nov 1, 2012
 *      Author: lutz
 */


/*
 * this is the communication interface to the power board
 * this interface is numbered as logic 2
 */

#define COM_2_PWR_BOARD_LOGIC_INTERFACE_NR 2U


#include <flawless/init/systemInitializer.h>
#include <flawless/protocol/genericFlawLessProtocol_Data.h>
#include <flawless/platform/system.h>

#include <libopencm3/stm32/f4/usart.h>
#include <libopencm3/stm32/f4/rcc.h>
#include <libopencm3/stm32/f4/gpio.h>
#include <libopencm3/stm32/f4/dma.h>
#include <libopencm3/stm32/f4/nvic.h>

#define TX_FIFO_SIZE 128U
#define RX_FIFO_SIZE 128U

#include <target/stm32f4/clock.h>

#define GEN_FIFO_USE_ATOMIC 1
#define GEN_FIFO_CLI_FUNCTION system_mutex_lock();
#define GEN_FIFO_SEI_FUNCTION system_mutex_unlock();
#include <flawless/misc/fifo/genericFiFo.h>

CREATE_GENERIC_FIFO(uint8_t, data, TX_FIFO_SIZE);



#define COM_2_PWR_USART_INTERFACE USART6
#define COM_2_PWR_USART_INTERFACE_SPEED 115200ULL

#define COM_2_PWR_USART_PORT GPIOC
#define COM_2_PWR_USART_TX_PIN  GPIO6
#define COM_2_PWR_USART_RX_PIN  GPIO7

#define COM_2_PWR_TX_DMA DMA2
#define COM_2_PWR_RX_DMA DMA2

#define COM_2_PWR_TX_DMA_STREAM DMA_STREAM_6
#define COM_2_PWR_RX_DMA_STREAM DMA_STREAM_1

#define COM_2_PWR_TX_DMA_CHANNEL 5U
#define COM_2_PWR_RX_DMA_CHANNEL 5U

static data_fifoHandle_t g_txFifo;

static flawLessTransportSymbol_t g_rxFifoBuf[RX_FIFO_SIZE];
static uint16_t g_rxFifoReadIndex = 0U;
static uint16_t g_currentTXTransactionSize = 0U;


static void trigger_transmision(void);
static void trigger_transmision(void)
{

	system_mutex_lock();
	uint16_t bytesToSendCurrently = MIN(TX_FIFO_SIZE - g_txFifo.start, g_txFifo.count);

	if ((bytesToSendCurrently > 0U) &&
		(0 == g_currentTXTransactionSize))
	{
		while (0 != (DMA_SCCR(COM_2_PWR_TX_DMA, COM_2_PWR_TX_DMA_STREAM) & DMA_CR_EN))
		{
			DMA_SCCR(COM_2_PWR_TX_DMA, COM_2_PWR_TX_DMA_STREAM) &= ~DMA_CR_EN;
		}
		/* set txBuffer as source */
		DMA_SM0AR(COM_2_PWR_TX_DMA, COM_2_PWR_TX_DMA_STREAM) = (uint32_t) &(g_txFifo.data[g_txFifo.start]);
		DMA_SNDTR(COM_2_PWR_TX_DMA, COM_2_PWR_TX_DMA_STREAM) = bytesToSendCurrently;
		g_currentTXTransactionSize = bytesToSendCurrently;

		DMA_SCCR(COM_2_PWR_TX_DMA, COM_2_PWR_TX_DMA_STREAM) |= DMA_CR_TCIE;
		DMA_SCCR(COM_2_PWR_TX_DMA, COM_2_PWR_TX_DMA_STREAM) |= DMA_CR_EN;
	}
	system_mutex_unlock();
}

/*
 * send function called from framework
 */
void phySendFunction2(const flawLessTransportSymbol_t *i_data, uint16_t i_packetLen)
{
	uint16_t packetBuffered = 0;
	while (packetBuffered < i_packetLen)
	{
		system_mutex_lock();
		/* how much of that packet can we send at once */
		uint16_t spaceFree = TX_FIFO_SIZE - g_txFifo.count;
		uint16_t spaceFreeTillWrap = MIN(TX_FIFO_SIZE - g_txFifo.end, spaceFree);
		uint16_t elementsToStore = MIN(i_packetLen - packetBuffered, spaceFreeTillWrap);
		memcpy(&(g_txFifo.data[g_txFifo.end]), &(i_data[packetBuffered]), elementsToStore);
		g_txFifo.end = (g_txFifo.end + elementsToStore) % TX_FIFO_SIZE;
		g_txFifo.count += elementsToStore;
		packetBuffered += elementsToStore;
		system_mutex_unlock();

		/* check if dma is allready running if not enable dma */
		trigger_transmision();
	}
}

/*
 * ISR for USART6 here we receive Data
 */
void usart6_isr(void)
{
	/* did we receive something? */
	while (g_rxFifoReadIndex != (RX_FIFO_SIZE - DMA_SNDTR(COM_2_PWR_RX_DMA, COM_2_PWR_RX_DMA_STREAM)))
	{
		uint8_t received = g_rxFifoBuf[g_rxFifoReadIndex];
		g_rxFifoReadIndex = (g_rxFifoReadIndex + 1) % RX_FIFO_SIZE;
		flawLess_ReceiveIndication(COM_2_PWR_BOARD_LOGIC_INTERFACE_NR, &received, sizeof(received));
	}
	if ((g_rxFifoReadIndex == (RX_FIFO_SIZE - DMA_SNDTR(COM_2_PWR_RX_DMA, COM_2_PWR_RX_DMA_STREAM))) &&
		(0 != (USART_SR(COM_2_PWR_USART_INTERFACE) & USART_SR_ORE)))
	{
		/* dummy read on usart interface */
		(void)USART_DR(COM_2_PWR_USART_INTERFACE);
	}
}


void dma2_stream6_isr(void)
{
	(void)DMA_HISR(COM_2_PWR_TX_DMA);
	DMA_HIFCR(COM_2_PWR_TX_DMA) = (0x3d << 16);
	system_mutex_lock();
	g_txFifo.start = (g_txFifo.start + g_currentTXTransactionSize) % TX_FIFO_SIZE;
	g_txFifo.count = g_txFifo.count - g_currentTXTransactionSize;
	g_currentTXTransactionSize = 0;
	system_mutex_unlock();
	trigger_transmision();
}


static void comm2pwrBrd_init(void);
MODULE_INIT_FUNCTION(comm2pwrBrd, 3, comm2pwrBrd_init);
static void comm2pwrBrd_init(void)
{
	data_init(&g_txFifo);
	data_peek(&g_txFifo, 0U, NULL); /* dummy call to suppress compiler warnings */
	data_getCount(&g_txFifo);       /* dummy call to suppress compiler warnings */
	data_put(NULL, NULL);       /* dummy call to suppress compiler warnings */
	data_get(NULL, NULL);       /* dummy call to suppress compiler warnings */

	memset(&g_rxFifoBuf, 0, sizeof(g_rxFifoBuf));
	g_rxFifoReadIndex = 0U;

	/* setup USART for communication */

	RCC_APB2ENR |= RCC_APB2ENR_USART6EN;
	RCC_AHB1ENR |= RCC_AHB1ENR_IOPCEN;
	RCC_AHB1ENR |= RCC_AHB1ENR_DMA2EN;

	gpio_mode_setup(COM_2_PWR_USART_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, COM_2_PWR_USART_TX_PIN);
	gpio_set_output_options(COM_2_PWR_USART_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_100MHZ, COM_2_PWR_USART_TX_PIN);
	gpio_set_af(COM_2_PWR_USART_PORT, GPIO_AF8, COM_2_PWR_USART_TX_PIN);

	gpio_mode_setup(COM_2_PWR_USART_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, COM_2_PWR_USART_RX_PIN);
	gpio_set_af(COM_2_PWR_USART_PORT, GPIO_AF8, COM_2_PWR_USART_RX_PIN);

	nvic_enable_irq(NVIC_USART6_IRQ);
	nvic_enable_irq(NVIC_DMA2_STREAM6_IRQ);

	USART_BRR(COM_2_PWR_USART_INTERFACE) = CLOCK_APB2_CLK / COM_2_PWR_USART_INTERFACE_SPEED;
	USART_CR1(COM_2_PWR_USART_INTERFACE) |= USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;
	USART_CR3(COM_2_PWR_USART_INTERFACE) |= USART_CR3_DMAT | USART_CR3_DMAR;

	/********* TX DMA **********/
	/* set channel */
	DMA_SCCR(COM_2_PWR_TX_DMA, COM_2_PWR_TX_DMA_STREAM)  = COM_2_PWR_TX_DMA_CHANNEL << DMA_CR_CHSEL_LSB | DMA_CR_DIR | DMA_CR_MINC | DMA_CR_TCIE;
	/* write to usart_dr */
	DMA_SPAR(COM_2_PWR_TX_DMA, COM_2_PWR_TX_DMA_STREAM)  = (uint32_t) &USART_DR(COM_2_PWR_USART_INTERFACE);

	/********* RX DMA **********/
	DMA_SCCR(COM_2_PWR_RX_DMA, COM_2_PWR_RX_DMA_STREAM)  = COM_2_PWR_RX_DMA_CHANNEL << DMA_CR_CHSEL_LSB | DMA_CR_MINC | DMA_CR_CIRC;
	/* read from usart_dr */
	DMA_SPAR(COM_2_PWR_RX_DMA, COM_2_PWR_RX_DMA_STREAM)  = (uint32_t) &USART_DR(COM_2_PWR_USART_INTERFACE);
	DMA_SM0AR(COM_2_PWR_RX_DMA, COM_2_PWR_RX_DMA_STREAM) = (uint32_t) &g_rxFifoBuf;
	DMA_SNDTR(COM_2_PWR_RX_DMA, COM_2_PWR_RX_DMA_STREAM) = RX_FIFO_SIZE;

	DMA_SCCR(COM_2_PWR_RX_DMA, COM_2_PWR_RX_DMA_STREAM) |= DMA_CR_EN;

	USART_CR1(COM_2_PWR_USART_INTERFACE) |= USART_CR1_UE;
}

