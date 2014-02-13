/*
 * com2MainComputer.c
 *
 *  Created on: Nov 1, 2012
 *      Author: lutz
 */


/*
 * this is the processor to processor communication
 * this interface is numbered as logic 1
 */

#define COM_2_MAIN_COMPUTER_LOGIC_INTERFACE_NR 1U


#include <flawless/init/systemInitializer.h>
#include <flawless/protocol/genericFlawLessProtocol_Data.h>
#include <flawless/core/msg_msgPump.h>
#include <flawless/timer/swTimer.h>

#include <framework/flawless/core/systemMsgIDs.h>
#include <framework/flawless/protocol/genericFlawLessProtocol.h>


#include <flawless/platform/system.h>

#include <libopencm3/stm32/f4/usart.h>
#include <libopencm3/stm32/f4/rcc.h>
#include <libopencm3/stm32/f4/gpio.h>
#include <libopencm3/stm32/f4/dma.h>
#include <libopencm3/stm32/nvic.h>

#include <target/stm32f4/clock.h>


#define GEN_FIFO_USE_ATOMIC 1

#define TX_FIFO_SIZE 128U

#define RX_FIFO_SIZE 512U

#define GEN_FIFO_CLI_FUNCTION system_mutex_lock()
#define GEN_FIFO_SEI_FUNCTION system_mutex_unlock()

#include <flawless/misc/fifo/genericFiFo.h>

CREATE_GENERIC_FIFO(uint8_t, com2main, TX_FIFO_SIZE)


#define COM_2_MAIN_USART_INTERFACE USART1
#define COM_2_MAIN_USART_INTERFACE_SPEED 921600ULL

#define COM_2_MAIN_USART_PORT GPIOA
#define COM_2_MAIN_USART_TX_PIN  GPIO9
#define COM_2_MAIN_USART_RX_PIN  GPIO10

#define COM_2_MAIN_TX_DMA DMA2
#define COM_2_MAIN_RX_DMA DMA2

#define COM_2_MAIN_TX_DMA_STREAM DMA_STREAM_7
#define COM_2_MAIN_RX_DMA_STREAM DMA_STREAM_5

#define COM_2_MAIN_TX_DMA_CHANNEL 4U
#define COM_2_MAIN_RX_DMA_CHANNEL 4U

#define COM_2_MAIN_DEFAULT_COMM_TIMEOUT_MS 3000U

static com2main_fifoHandle_t g_txFifo;

static flawLessTransportSymbol_t g_rxFifoBuf[RX_FIFO_SIZE];
static uint16_t g_rxFifoReadIndex = 0U;
static uint16_t g_currentTXTransactionSize = 0U;

static volatile bool g_txEnabled = false;

void phySendFunction1(const flawLessTransportSymbol_t *i_data, uint16_t i_packetLen);


static void onCommunicationTimeout();
static void onCommunicationTimeout()
{
	g_txEnabled = false;
}

static void onReceivedPacket(msgPump_MsgID_t msgID, const void *data);
static void onReceivedPacket(msgPump_MsgID_t msgID, const void *data)
{
	if (MSG_ID_GENERIC_PROTOCOL_PACKET_READY == msgID)
	{
		const genericProtocoll_Packet_t *packet = (const genericProtocoll_Packet_t*)data;
		if (COM_2_MAIN_COMPUTER_LOGIC_INTERFACE_NR == packet->interface)
		{
			g_txEnabled = true;
			swTimer_registerOnTimer(&onCommunicationTimeout, COM_2_MAIN_DEFAULT_COMM_TIMEOUT_MS, false);
		}
	}
}


static void trigger_transmision(void);
static void trigger_transmision(void)
{

	system_mutex_lock();
	uint16_t bytesToSendCurrently = MIN(TX_FIFO_SIZE - g_txFifo.start, g_txFifo.count);

	if ((bytesToSendCurrently > 0U) &&
		(0 == g_currentTXTransactionSize))
	{
		while (0 != (DMA_SCCR(COM_2_MAIN_TX_DMA, COM_2_MAIN_TX_DMA_STREAM) & DMA_CR_EN))
		{
			DMA_SCCR(COM_2_MAIN_TX_DMA, COM_2_MAIN_TX_DMA_STREAM) &= ~DMA_CR_EN;
		}

		/* set txBuffer as source */
		DMA_SM0AR(COM_2_MAIN_TX_DMA, COM_2_MAIN_TX_DMA_STREAM) = (uint32_t) &(g_txFifo.data[g_txFifo.start]);
		DMA_SNDTR(COM_2_MAIN_TX_DMA, COM_2_MAIN_TX_DMA_STREAM) = bytesToSendCurrently;
		g_currentTXTransactionSize = bytesToSendCurrently;

		DMA_SCCR(COM_2_MAIN_TX_DMA, COM_2_MAIN_TX_DMA_STREAM) |= DMA_CR_TCIE;
		DMA_SCCR(COM_2_MAIN_TX_DMA, COM_2_MAIN_TX_DMA_STREAM) |= DMA_CR_EN;
	}
	system_mutex_unlock();
}

/*
 * send function called from framework
 */
void phySendFunction1(const flawLessTransportSymbol_t *i_data, uint16_t i_packetLen)
{
	if (false != g_txEnabled)
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
}

/*
 * ISR for USART here we send AND receive Data
 */
void usart1_isr(void)
{
	/* did we receive something? */
	while (g_rxFifoReadIndex != (RX_FIFO_SIZE - DMA_SNDTR(COM_2_MAIN_RX_DMA, COM_2_MAIN_RX_DMA_STREAM)))
	{
		uint8_t received = g_rxFifoBuf[g_rxFifoReadIndex];
		g_rxFifoReadIndex = (g_rxFifoReadIndex + 1) % RX_FIFO_SIZE;
		flawLess_ReceiveIndication(COM_2_MAIN_COMPUTER_LOGIC_INTERFACE_NR, &received, sizeof(received));
	}
	if ((g_rxFifoReadIndex == (RX_FIFO_SIZE - DMA_SNDTR(COM_2_MAIN_RX_DMA, COM_2_MAIN_RX_DMA_STREAM))) &&
		(0 != (USART_SR(COM_2_MAIN_USART_INTERFACE) & USART_SR_ORE)))
	{
		/* dummy read on usart interface */
		(void)USART_DR(COM_2_MAIN_USART_INTERFACE);
	}
}

void dma2_stream7_isr(void)
{
	(void)DMA_HISR(COM_2_MAIN_TX_DMA);
	DMA_HIFCR(COM_2_MAIN_TX_DMA) = (0x3d << 22);
	system_mutex_lock();
	g_txFifo.start = (g_txFifo.start + g_currentTXTransactionSize) % TX_FIFO_SIZE;
	g_txFifo.count = g_txFifo.count - g_currentTXTransactionSize;
	g_currentTXTransactionSize = 0;
	system_mutex_unlock();
	trigger_transmision();
}

static void com_2_main_init(void);
MODULE_INIT_FUNCTION(com2main, 5, com_2_main_init)
static void com_2_main_init(void)
{
	g_txEnabled = false;

	com2main_init(&g_txFifo);
	com2main_peek(&g_txFifo, 0U, NULL); /* dummy call to suppress compiler warnings */
	com2main_getCount(&g_txFifo);       /* dummy call to suppress compiler warnings */
	com2main_put(NULL, NULL);       /* dummy call to suppress compiler warnings */
	com2main_get(NULL, NULL);       /* dummy call to suppress compiler warnings */

	memset(&g_rxFifoBuf, 0, sizeof(g_rxFifoBuf));
	g_rxFifoReadIndex = 0U;

	/* setup USART for communication */

	RCC_APB2ENR |= RCC_APB2ENR_USART1EN;
	RCC_AHB1ENR |= RCC_AHB1ENR_IOPAEN;
	RCC_AHB1ENR |= RCC_AHB1ENR_DMA2EN;

	gpio_set_af(COM_2_MAIN_USART_PORT, GPIO_AF7, COM_2_MAIN_USART_TX_PIN);
	gpio_mode_setup(COM_2_MAIN_USART_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, COM_2_MAIN_USART_TX_PIN);
	gpio_set_output_options(COM_2_MAIN_USART_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_100MHZ, COM_2_MAIN_USART_TX_PIN);

	gpio_mode_setup(COM_2_MAIN_USART_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, COM_2_MAIN_USART_RX_PIN);
	gpio_set_af(COM_2_MAIN_USART_PORT, GPIO_AF7, COM_2_MAIN_USART_RX_PIN);

	nvic_enable_irq(NVIC_USART1_IRQ);
	nvic_enable_irq(NVIC_DMA2_STREAM7_IRQ);

	USART_BRR(COM_2_MAIN_USART_INTERFACE) = CLOCK_APB2_CLK / COM_2_MAIN_USART_INTERFACE_SPEED;
	USART_CR1(COM_2_MAIN_USART_INTERFACE) |= USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;
	USART_CR3(COM_2_MAIN_USART_INTERFACE) |= USART_CR3_DMAT | USART_CR3_DMAR;
	USART_CR1(COM_2_MAIN_USART_INTERFACE) |= USART_CR1_UE;


	/********* TX DMA **********/
	/* set channel */
	DMA_SCCR(COM_2_MAIN_TX_DMA, COM_2_MAIN_TX_DMA_STREAM)  = COM_2_MAIN_TX_DMA_CHANNEL << DMA_CR_CHSEL_LSB | DMA_CR_DIR | DMA_CR_MINC | DMA_CR_TCIE;
	/* write to usart_dr */
	DMA_SPAR(COM_2_MAIN_TX_DMA, COM_2_MAIN_TX_DMA_STREAM)  = (uint32_t) &USART_DR(COM_2_MAIN_USART_INTERFACE);

	/********* RX DMA **********/
	DMA_SCCR(COM_2_MAIN_RX_DMA, COM_2_MAIN_RX_DMA_STREAM)  = COM_2_MAIN_RX_DMA_CHANNEL << DMA_CR_CHSEL_LSB | DMA_CR_MINC | DMA_CR_TCIE | DMA_CR_CIRC;
	/* read from usart_dr */
	DMA_SPAR(COM_2_MAIN_RX_DMA, COM_2_MAIN_RX_DMA_STREAM)  = (uint32_t) &USART_DR(COM_2_MAIN_USART_INTERFACE);
	DMA_SM0AR(COM_2_MAIN_RX_DMA, COM_2_MAIN_RX_DMA_STREAM) = (uint32_t) &g_rxFifoBuf;
	DMA_SNDTR(COM_2_MAIN_RX_DMA, COM_2_MAIN_RX_DMA_STREAM) = RX_FIFO_SIZE;

	DMA_SCCR(COM_2_MAIN_RX_DMA, COM_2_MAIN_RX_DMA_STREAM) |= DMA_CR_EN;

	msgPump_registerOnMessage(MSG_ID_GENERIC_PROTOCOL_PACKET_READY, &onReceivedPacket);
}
