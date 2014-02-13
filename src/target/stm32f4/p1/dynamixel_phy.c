/*
 * dynamixel.c
 *
 *  Created on: Dec 1, 2012
 *      Author: lutz
 */


#include <flawless/init/systemInitializer.h>
#include <flawless/platform/system.h>

#include <libopencm3/stm32/f4/gpio.h>
#include <libopencm3/stm32/f4/rcc.h>
#include <libopencm3/stm32/f4/usart.h>
#include <libopencm3/stm32/f4/nvic_f4.h>
#include <libopencm3/stm32/nvic.h>

#include <libopencm3/stm32/f4/dma.h>

#include <target/stm32f4/clock.h>

#include "dynamixel_phy.h"

#define DYNAMIXEL_PHY_BAUDRATE 1000000ULL


typedef enum tagPortRTxMode
{
	PORT_MODE_IDLE = 0U,
	PORT_MODE_RECEIVING = 1U,
	PORT_MODE_TRANSMITTING = 2U
} portRTxMode;

typedef struct portMode
{
	const uint8_t *txBuf;
	uint8_t *rxBuf;
	portRTxMode rtxMode;
	uint8_t bytesToSend;
	uint8_t bytesSent;

	uint8_t bytesToReceive;
	uint8_t bytesReceived;
} portMode_t;



static portMode_t g_portModes[DYNAMIXEL_PORT_CNT];
static dynamixel_txComplete_callback_t g_txCompleteCallback = NULL;
static dynamixel_receive_callback_t    g_receiveCallback = NULL;



typedef struct tag_rs485_interface
{
	uint32_t TXE_Port, RXE_Port;          /* the phy port to controll the dataflow direction */
	uint32_t serialInterface;             /* which UART */
	uint32_t serialTXPort, serialRXPort;
	uint32_t dma;                         /* which dma to use */
	uint8_t dmaTXChannel;                   /* dma channel to use (TX) */
	uint8_t dmaTXStream;                    /* dma stream to use (TX) */
	uint8_t dmaRXChannel;                   /* dma channel to use (RX) */
	uint8_t dmaRXStream;                    /* dma stream to use (RX) */
	uint16_t  TX_pin, RX_pin;
	uint16_t  TXE_Pin, RXE_Pin;           /* which pins controll the dataflow direction */
} rs485_interface_t;


static const rs485_interface_t g_interfaces[] = {
		{
				GPIOC, GPIOC,
				UART5,
				GPIOC, GPIOD,
				DMA1,
				4,
				DMA_STREAM_7,
				4,
				DMA_STREAM_0,
				GPIO12, GPIO2,
				GPIO8, GPIO7
		},
		{
				GPIOC, GPIOB,
				USART3,
				GPIOB, GPIOB,
				DMA1,
				4,
				DMA_STREAM_3,
				4,
				DMA_STREAM_1,
				GPIO10, GPIO11,
				GPIO5, GPIO1
		},
		{
				GPIOC, GPIOC,
				UART4,
				GPIOC, GPIOC,
				DMA1,
				4,
				DMA_STREAM_4,
				4,
				DMA_STREAM_2,
				GPIO10, GPIO11,
				GPIO0, GPIO1
		}
};

static void enableClockForUART(const uint32_t i_uart)
{
	switch (i_uart)
	{
		case USART1:
			RCC_APB2ENR |= RCC_APB2ENR_USART1EN;
			break;
		case USART6:
			RCC_APB2ENR |= RCC_APB2ENR_USART6EN;
			break;
		case USART2:
			RCC_APB1ENR |= RCC_APB1ENR_USART2EN;
			break;
		case USART3:
			RCC_APB1ENR |= RCC_APB1ENR_USART3EN;
			break;
		case UART4:
			RCC_APB1ENR |= RCC_APB1ENR_UART4EN;
			break;
		case UART5:
			RCC_APB1ENR |= RCC_APB1ENR_UART5EN;
			break;
		default:
			break;
	}
}

static void setupUART(const uint32_t i_uart)
{
	USART_CR1(i_uart) |= USART_CR1_UE;
	switch (i_uart)
	{
		case USART1:
		case USART6:
			USART_BRR(i_uart) = CLOCK_APB2_CLK / DYNAMIXEL_PHY_BAUDRATE;
			break;
		case USART2:
		case USART3:
		case UART4:
		case UART5:
			USART_BRR(i_uart) = CLOCK_APB1_CLK / DYNAMIXEL_PHY_BAUDRATE;
			break;
		default:
			break;
	}
	USART_CR1(i_uart) |= USART_CR1_TE | USART_CR1_RE;
}


static void enableClockForPort(const uint32_t i_port)
{
	switch (i_port)
	{
		case GPIOA:
			RCC_AHB1ENR |= RCC_AHB1ENR_IOPAEN;
			break;
		case GPIOB:
			RCC_AHB1ENR |= RCC_AHB1ENR_IOPBEN;
			break;
		case GPIOC:
			RCC_AHB1ENR |= RCC_AHB1ENR_IOPCEN;
			break;
		case GPIOD:
			RCC_AHB1ENR |= RCC_AHB1ENR_IOPDEN;
			break;
		case GPIOE:
			RCC_AHB1ENR |= RCC_AHB1ENR_IOPEEN;
			break;
		case GPIOF:
			RCC_AHB1ENR |= RCC_AHB1ENR_IOPFEN;
			break;
		case GPIOG:
			RCC_AHB1ENR |= RCC_AHB1ENR_IOPGEN;
			break;
		case GPIOH:
			RCC_AHB1ENR |= RCC_AHB1ENR_IOPHEN;
			break;
		case GPIOI:
			RCC_AHB1ENR |= RCC_AHB1ENR_IOPIEN;
			break;
	}
}


static void enableClockForDMA(const uint32_t i_dma)
{
	switch (i_dma)
	{
		case DMA1:
			RCC_AHB1ENR |= RCC_AHB1ENR_DMA1EN;
			break;
		case DMA2:
			RCC_AHB1ENR |= RCC_AHB1ENR_DMA2EN;
			break;
		default:
			break;
	}
}

static void enableDMAInterupt(uint32_t dma, uint8_t stream)
{

	if (DMA1 == dma)
	{
		switch (stream)
		{
			case DMA_STREAM_0:
				nvic_enable_irq(NVIC_DMA1_STREAM0_IRQ);
				nvic_set_priority(NVIC_DMA1_STREAM0_IRQ, 5);
				break;
			case DMA_STREAM_1:
				nvic_enable_irq(NVIC_DMA1_STREAM1_IRQ);
				nvic_set_priority(NVIC_DMA1_STREAM1_IRQ, 5);
				break;
			case DMA_STREAM_2:
				nvic_enable_irq(NVIC_DMA1_STREAM2_IRQ);
				nvic_set_priority(NVIC_DMA1_STREAM2_IRQ, 5);
				break;
			case DMA_STREAM_3:
				nvic_enable_irq(NVIC_DMA1_STREAM3_IRQ);
				nvic_set_priority(NVIC_DMA1_STREAM3_IRQ, 5);
				break;
			case DMA_STREAM_4:
				nvic_enable_irq(NVIC_DMA1_STREAM4_IRQ);
				nvic_set_priority(NVIC_DMA1_STREAM4_IRQ, 5);
				break;
			case DMA_STREAM_5:
				nvic_enable_irq(NVIC_DMA1_STREAM5_IRQ);
				nvic_set_priority(NVIC_DMA1_STREAM5_IRQ, 5);
				break;
			case DMA_STREAM_6:
				nvic_enable_irq(NVIC_DMA1_STREAM6_IRQ);
				nvic_set_priority(NVIC_DMA1_STREAM6_IRQ, 5);
				break;
			case DMA_STREAM_7:
				nvic_enable_irq(NVIC_DMA1_STREAM7_IRQ);
				nvic_set_priority(NVIC_DMA1_STREAM7_IRQ, 5);
				break;
				default:
					break;
		}
	} else if (DMA2 == dma)
	{

		switch (stream)
		{
			case DMA_STREAM_0:
				nvic_enable_irq(NVIC_DMA2_STREAM0_IRQ);
				nvic_set_priority(NVIC_DMA2_STREAM0_IRQ, 5);
				break;
			case DMA_STREAM_1:
				nvic_enable_irq(NVIC_DMA2_STREAM1_IRQ);
				nvic_set_priority(NVIC_DMA2_STREAM1_IRQ, 5);
				break;
			case DMA_STREAM_2:
				nvic_enable_irq(NVIC_DMA2_STREAM2_IRQ);
				nvic_set_priority(NVIC_DMA2_STREAM2_IRQ, 5);
				break;
			case DMA_STREAM_3:
				nvic_enable_irq(NVIC_DMA2_STREAM3_IRQ);
				nvic_set_priority(NVIC_DMA2_STREAM3_IRQ, 5);
				break;
			case DMA_STREAM_4:
				nvic_enable_irq(NVIC_DMA2_STREAM4_IRQ);
				nvic_set_priority(NVIC_DMA2_STREAM4_IRQ, 5);
				break;
			case DMA_STREAM_5:
				nvic_enable_irq(NVIC_DMA2_STREAM5_IRQ);
				nvic_set_priority(NVIC_DMA2_STREAM5_IRQ, 5);
				break;
			case DMA_STREAM_6:
				nvic_enable_irq(NVIC_DMA2_STREAM6_IRQ);
				nvic_set_priority(NVIC_DMA2_STREAM6_IRQ, 5);
				break;
			case DMA_STREAM_7:
				nvic_enable_irq(NVIC_DMA2_STREAM7_IRQ);
				nvic_set_priority(NVIC_DMA2_STREAM7_IRQ, 5);
				break;
				default:
					break;
		}
	}
}

static void setupDMA(const rs485_interface_t *i_interface)
{
	DMA_SCCR(i_interface->dma, i_interface->dmaRXStream) = 0;
	DMA_SCCR(i_interface->dma, i_interface->dmaTXStream) = 0;

	DMA_SCCR(i_interface->dma, i_interface->dmaRXStream) = (i_interface->dmaRXChannel << 25) | (DMA_CR_PL_HIGH) | (0 << DMA_CR_DIR_SHIFT) | DMA_CR_MSIZE_BYTE | DMA_CR_PSIZE_BYTE  | (DMA_CR_MINC) | (DMA_CR_TCIE);
	DMA_SCCR(i_interface->dma, i_interface->dmaTXStream) = (i_interface->dmaTXChannel << 25) | (DMA_CR_PL_HIGH) | (1 << DMA_CR_DIR_SHIFT) | DMA_CR_MSIZE_BYTE | DMA_CR_PSIZE_BYTE | (DMA_CR_MINC);
	enableDMAInterupt(i_interface->dma, i_interface->dmaRXStream);
}

static void setupInterface(const rs485_interface_t *i_interface)
{
	uint8_t alternateFunction = 0U;

	/* enable periphery */
	enableClockForUART(i_interface->serialInterface);
	enableClockForPort(i_interface->RXE_Port);
	enableClockForPort(i_interface->TXE_Port);
	enableClockForPort(i_interface->serialTXPort);
	enableClockForPort(i_interface->serialRXPort);
	enableClockForDMA(i_interface->dma);

	gpio_mode_setup(i_interface->TXE_Port, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, i_interface->TXE_Pin);
	gpio_set_output_options(i_interface->TXE_Port, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, i_interface->TXE_Pin);
	gpio_clear(i_interface->TXE_Port, i_interface->TXE_Pin);

	gpio_mode_setup(i_interface->RXE_Port, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, i_interface->RXE_Pin);
	gpio_set_output_options(i_interface->RXE_Port, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, i_interface->RXE_Pin);
	gpio_clear(i_interface->RXE_Port, i_interface->RXE_Pin);

	gpio_mode_setup(i_interface->serialTXPort, GPIO_MODE_AF, GPIO_PUPD_NONE, i_interface->TX_pin);
	gpio_set_output_options(i_interface->serialTXPort, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, i_interface->TX_pin);

	gpio_mode_setup(i_interface->serialRXPort, GPIO_MODE_AF, GPIO_PUPD_PULLUP, i_interface->RX_pin);

	switch (i_interface->serialInterface)
	{
		case USART1:
			alternateFunction = GPIO_AF7;
			nvic_enable_irq(NVIC_USART1_IRQ);
			nvic_set_priority(NVIC_USART1_IRQ, 1);
			break;
		case USART2:
			alternateFunction = GPIO_AF7;
			nvic_enable_irq(NVIC_USART2_IRQ);
			nvic_set_priority(NVIC_USART2_IRQ, 1);
			break;
		case USART3:
			alternateFunction = GPIO_AF7;
			nvic_enable_irq(NVIC_USART3_IRQ);
			nvic_set_priority(NVIC_USART3_IRQ, 1);
			break;
		case UART4:
			alternateFunction = GPIO_AF8;
			nvic_enable_irq(NVIC_UART4_IRQ);
			nvic_set_priority(NVIC_UART4_IRQ, 1);
			break;
		case UART5:
			alternateFunction = GPIO_AF8;
			nvic_enable_irq(NVIC_UART5_IRQ);
			nvic_set_priority(NVIC_UART5_IRQ, 1);
			break;
		case USART6:
			alternateFunction = GPIO_AF8;
			nvic_enable_irq(NVIC_USART6_IRQ);
			nvic_set_priority(NVIC_USART6_IRQ, 1);
			break;
	}

	gpio_set_af(i_interface->serialTXPort, alternateFunction, i_interface->TX_pin);
	gpio_set_af(i_interface->serialRXPort, alternateFunction, i_interface->RX_pin);
	setupUART(i_interface->serialInterface);
	setupDMA(i_interface);
}

void dynamixel_phy_setInterfaceMode(dynamixel_port_t port, dynamixel_interfaceMode_t mode)
{
	if (port < DYNAMIXEL_PORT_CNT)
	{
		const rs485_interface_t *interface = &(g_interfaces[port]);
		if (DYNAMIXEL_INTERFACE_MODE_RX == mode)
		{
			gpio_clear(interface->RXE_Port, interface->RXE_Pin);
			gpio_clear(interface->TXE_Port, interface->TXE_Pin);
		} else if (DYNAMIXEL_INTERFACE_MODE_TX == mode)
		{
			gpio_set(interface->RXE_Port, interface->RXE_Pin);
			gpio_set(interface->TXE_Port, interface->TXE_Pin);
		}
	}
}


void dynamixel_phy_setRxCompleteCallback(dynamixel_receive_callback_t i_callback)
{
	g_receiveCallback = i_callback;
}

void dynamixel_phy_setTXCCallback(dynamixel_txComplete_callback_t i_callback)
{
	g_txCompleteCallback = i_callback;
}

void dynamixel_phy_resetInterface(dynamixel_port_t port)
{
	if (port < DYNAMIXEL_PORT_CNT)
	{
		system_mutex_lock();

		const rs485_interface_t *interface = &(g_interfaces[port]);
		USART_CR1(interface->serialInterface) &= ~USART_CR1_TCIE;

		/* disable tx and rx dma */
		DMA_SCCR(interface->dma, interface->dmaRXStream) &= ~DMA_CR_TCIE;
		DMA_SCCR(interface->dma, interface->dmaTXStream) &= ~DMA_CR_TCIE;
		DMA_SCCR(interface->dma, interface->dmaRXStream) &= ~DMA_CR_EN;
		DMA_SCCR(interface->dma, interface->dmaTXStream) &= ~DMA_CR_EN;

		switch (interface->dmaRXStream)
		{
			case DMA_STREAM_0:
				DMA_LIFCR(interface->dma) = (0x3d << 0);
				break;
			case DMA_STREAM_1:
				DMA_LIFCR(interface->dma) = (0x3d << 6);
				break;
			case DMA_STREAM_2:
				DMA_LIFCR(interface->dma) = (0x3d << 16);
				break;
			case DMA_STREAM_3:
				DMA_LIFCR(interface->dma) = (0x3d << 22);
				break;
			case DMA_STREAM_4:
				DMA_HIFCR(interface->dma) = (0x3d << 0);
				break;
			case DMA_STREAM_5:
				DMA_HIFCR(interface->dma) = (0x3d << 6);
				break;
			case DMA_STREAM_6:
				DMA_HIFCR(interface->dma) = (0x3d << 16);
				break;
			case DMA_STREAM_7:
				DMA_HIFCR(interface->dma) = (0x3d << 22);
				break;
		}

		g_portModes[port].rtxMode = PORT_MODE_IDLE;
		g_portModes[port].bytesReceived  = 0U;
		g_portModes[port].bytesToReceive = 0U;
		g_portModes[port].bytesToSend    = 0U;
		g_portModes[port].rxBuf          = NULL;
		g_portModes[port].txBuf          = NULL;


		system_mutex_unlock();
	}else
	{
		system_mutex_lock();
		system_mutex_unlock();
	}
}

#include "interfaces/systemTime.h"

bool dynamixel_phy_send(dynamixel_port_t port, uint8_t *i_data, uint8_t len)
{
	bool ret = false;
	if (DYNAMIXEL_PORT_CNT > port)
	{
		portMode_t *mode = &g_portModes[port];
		const rs485_interface_t *interface = &(g_interfaces[port]);

		system_mutex_lock();
		if (PORT_MODE_IDLE == mode->rtxMode)
		{
			mode->rtxMode = PORT_MODE_TRANSMITTING;
			dynamixel_phy_setInterfaceMode(port, DYNAMIXEL_INTERFACE_MODE_TX);

			/* disable dma channel */
			DMA_SCCR(interface->dma, interface->dmaTXStream) &= ~DMA_CR_EN;
			while (0 != (DMA_SCCR(interface->dma, interface->dmaTXStream) & DMA_CR_EN));

			USART_CR3(interface->serialInterface) |= USART_CR3_DMAT;

			DMA_SPAR(interface->dma, interface->dmaTXStream) = (uint32_t)&(USART_DR(interface->serialInterface));
			DMA_SM0AR(interface->dma, interface->dmaTXStream) = (uint32_t)i_data;
			DMA_SNDTR(interface->dma, interface->dmaTXStream) = len;

			USART_SR(interface->serialInterface) &= ~(USART_SR_TC);
			USART_CR1(interface->serialInterface) |= USART_CR1_TCIE;

			DMA_SFCR(interface->dma, interface->dmaTXStream) |= DMA_FCR_FEIE;
			DMA_SCCR(interface->dma, interface->dmaTXStream) |= (DMA_CR_TCIE) | DMA_CR_DMEIE;
			DMA_SCCR(interface->dma, interface->dmaTXStream) |= (DMA_CR_EN);

			(void)DMA_LISR(interface->dma);
			(void)DMA_SFCR(interface->dma, interface->dmaTXStream);
			(void)DMA_HISR(interface->dma);
			ret = true;
		}
		system_mutex_unlock();
	}
	return ret;
}

bool dynamixel_phy_receive(dynamixel_port_t port, uint8_t *i_data, uint8_t i_len)
{
	bool ret = false;
	if (DYNAMIXEL_PORT_CNT > port)
	{
		portMode_t *mode = &g_portModes[port];
		const rs485_interface_t *interface = &(g_interfaces[port]);

		system_mutex_lock();
		if (PORT_MODE_IDLE == mode->rtxMode)
		{

			if (0 != (USART_SR(interface->serialInterface) & (USART_SR_ORE | USART_SR_RXNE)))
			{
				const uint8_t dummy = USART_DR(interface->serialInterface);
				UNUSED(dummy);
			}

			switch (interface->dmaRXStream)
			{
				case DMA_STREAM_0:
					DMA_LIFCR(interface->dma) = (0x3d << 0);
					break;
				case DMA_STREAM_1:
					DMA_LIFCR(interface->dma) = (0x3d << 6);
					break;
				case DMA_STREAM_2:
					DMA_LIFCR(interface->dma) = (0x3d << 16);
					break;
				case DMA_STREAM_3:
					DMA_LIFCR(interface->dma) = (0x3d << 22);
					break;
				case DMA_STREAM_4:
					DMA_HIFCR(interface->dma) = (0x3d << 0);
					break;
				case DMA_STREAM_5:
					DMA_HIFCR(interface->dma) = (0x3d << 6);
					break;
				case DMA_STREAM_6:
					DMA_HIFCR(interface->dma) = (0x3d << 16);
					break;
				case DMA_STREAM_7:
					DMA_HIFCR(interface->dma) = (0x3d << 22);
					break;
			}

			mode->rtxMode = PORT_MODE_RECEIVING;
			dynamixel_phy_setInterfaceMode(port, DYNAMIXEL_INTERFACE_MODE_RX);

			DMA_SCCR(interface->dma, interface->dmaRXStream) &= ~(DMA_CR_EN);
			while (0 != (DMA_SCCR(interface->dma, interface->dmaTXStream) & DMA_CR_EN));

			USART_CR3(interface->serialInterface) |= USART_CR3_DMAR;

			DMA_SPAR(interface->dma, interface->dmaRXStream) = (uint32_t)&(USART_DR(interface->serialInterface));
			DMA_SM0AR(interface->dma, interface->dmaRXStream) = (uint32_t)i_data;

			mode->rxBuf = i_data;
			mode->bytesReceived = 0U;
			mode->bytesToReceive = i_len;

			DMA_SNDTR(interface->dma, interface->dmaRXStream) = i_len;

			DMA_SCCR(interface->dma, interface->dmaRXStream) |= (DMA_CR_TCIE);
			DMA_SCCR(interface->dma, interface->dmaRXStream) |= (DMA_CR_EN);

			ret = true;
			(void)DMA_LISR(DMA1);
			(void)DMA_HISR(DMA1);
		}
		system_mutex_unlock();
	}
	return ret;
}


/* rx ISRs */
void dma1_stream0_isr(void)
{
	dynamixel_port_t port = DYNAMIXEL_PORT_1;
	portMode_t *mode = &g_portModes[port];
	const rs485_interface_t *interface = &(g_interfaces[port]);

	USART_CR3(interface->serialInterface) &= ~USART_CR3_DMAR;

	mode->rtxMode = PORT_MODE_IDLE;

	if (0 != (DMA_LISR(interface->dma) & DMA_LISR_TCIF0))
	{
		DMA_LIFCR(interface->dma) = (DMA_LISR_TCIF0) | (DMA_LISR_HTIF0) | (DMA_LISR_TEIF0) | (DMA_LISR_DMEIF0) | (DMA_LISR_TCIF0) | (DMA_LISR_FEIF0);
		if(NULL != g_receiveCallback)
		{
			mode->bytesReceived = mode->bytesToReceive - DMA_SNDTR(interface->dma, interface->dmaRXStream);
			(void)(g_receiveCallback)(port, mode->rxBuf, mode->bytesReceived);
		}
	}
}

void dma1_stream1_isr(void)
{
	dynamixel_port_t port = DYNAMIXEL_PORT_2;
	portMode_t *mode = &g_portModes[port];
	const rs485_interface_t *interface = &(g_interfaces[port]);

	USART_CR3(interface->serialInterface) &= ~USART_CR3_DMAR;

	mode->rtxMode = PORT_MODE_IDLE;

	if (0 != (DMA_LISR(interface->dma) & DMA_LISR_TCIF1))
	{
		DMA_LIFCR(interface->dma) = (DMA_LISR_TCIF1) | (DMA_LISR_HTIF1) | (DMA_LISR_TEIF1) | (DMA_LISR_DMEIF1) | (DMA_LISR_TCIF1) | (DMA_LISR_FEIF1);
		if(NULL != g_receiveCallback)
		{
			mode->bytesReceived = mode->bytesToReceive - DMA_SNDTR(interface->dma, interface->dmaRXStream);
			(void)(g_receiveCallback)(port, mode->rxBuf, mode->bytesReceived);
		}
	}

	if (0 != (DMA_LISR(interface->dma) & (DMA_LISR_FEIF1 | DMA_LISR_DMEIF1)))
	{
		DMA_LIFCR(interface->dma) = (DMA_LISR_TCIF1) | (DMA_LISR_HTIF1) | (DMA_LISR_TEIF1) | (DMA_LISR_DMEIF1) | (DMA_LISR_TCIF1) | (DMA_LISR_FEIF1);
	}
}

void dma1_stream2_isr(void)
{
	dynamixel_port_t port = DYNAMIXEL_PORT_3;
	portMode_t *mode = &g_portModes[port];
	const rs485_interface_t *interface = &(g_interfaces[port]);

	USART_CR3(interface->serialInterface) &= ~USART_CR3_DMAR;

	mode->rtxMode = PORT_MODE_IDLE;

	if (0 != (DMA_LISR(interface->dma) & DMA_LISR_TCIF2))
	{
		DMA_LIFCR(interface->dma) = (DMA_LISR_TCIF2) | (DMA_LISR_HTIF2) | (DMA_LISR_TEIF2) | (DMA_LISR_DMEIF2) | (DMA_LISR_TCIF2) | (DMA_LISR_FEIF2);
		if(NULL != g_receiveCallback)
		{
			mode->bytesReceived = mode->bytesToReceive - DMA_SNDTR(interface->dma, interface->dmaRXStream);
			(void)(g_receiveCallback)(port, mode->rxBuf, mode->bytesReceived);
		}
	}
}

/* tx ISRs */
void usart3_isr(void)
{
	dynamixel_port_t port = DYNAMIXEL_PORT_2;
	portMode_t *mode = &g_portModes[port];
	const rs485_interface_t *interface = &(g_interfaces[port]);

	if (0 != (USART_SR(interface->serialInterface) & (USART_SR_ORE | USART_SR_RXNE)))
	{
		uint8_t dummy = USART_DR(interface->serialInterface);
		UNUSED(dummy);
	}

	if (0 != (USART_SR(interface->serialInterface) & USART_SR_TC))
	{
		dynamixel_phy_setInterfaceMode(port, DYNAMIXEL_INTERFACE_MODE_RX);

		mode->rtxMode = PORT_MODE_IDLE;
		USART_CR1(interface->serialInterface) &= ~USART_CR1_TCIE;
		USART_CR3(interface->serialInterface) &= ~USART_CR3_DMAT;

		/* clear interrupt pendig flag of corresponding DMA */
		DMA_LIFCR(interface->dma) = (DMA_LISR_TCIF3) | (DMA_LISR_HTIF3) | (DMA_LISR_TEIF3) | (DMA_LISR_DMEIF3) | (DMA_LISR_TCIF3) | (DMA_LISR_FEIF3);


		/* tell the world! */
		if (NULL != g_txCompleteCallback)
		{
			(void)(g_txCompleteCallback)(port);
		}
	}
}

void uart4_isr(void)
{
	dynamixel_port_t port = DYNAMIXEL_PORT_3;
	portMode_t *mode = &g_portModes[port];
	const rs485_interface_t *interface = &(g_interfaces[port]);

	if (0 != (USART_SR(interface->serialInterface) & (USART_SR_ORE | USART_SR_RXNE)))
	{
		uint8_t dummy = USART_DR(interface->serialInterface);
		UNUSED(dummy);
	}

	if (0 != (USART_SR(interface->serialInterface) & USART_SR_TC))
	{
		dynamixel_phy_setInterfaceMode(port, DYNAMIXEL_INTERFACE_MODE_RX);

		mode->rtxMode = PORT_MODE_IDLE;
		USART_CR1(interface->serialInterface) &= ~USART_CR1_TCIE;
		USART_CR3(interface->serialInterface) &= ~USART_CR3_DMAT;

		/* clear interrupt pendig flag of corresponding DMA */
		DMA_HIFCR(interface->dma) = (DMA_HISR_TCIF4) | (DMA_HISR_HTIF4) | (DMA_HISR_TEIF4) | (DMA_HISR_DMEIF4) | (DMA_HISR_TCIF4) | (DMA_HISR_FEIF4);

		/* tell the world! */
		if (NULL != g_txCompleteCallback)
		{
			(void)(g_txCompleteCallback)(port);
		}
	}
}

void uart5_isr(void)
{
	dynamixel_port_t port = DYNAMIXEL_PORT_1;
	portMode_t *mode = &g_portModes[port];
	const rs485_interface_t *interface = &(g_interfaces[port]);

	if (0 != (USART_SR(interface->serialInterface) & (USART_SR_ORE | USART_SR_RXNE)))
	{
		uint8_t dummy = USART_DR(interface->serialInterface);
		UNUSED(dummy);
	}

	if (0 != (USART_SR(interface->serialInterface) & USART_SR_TC))
	{
		dynamixel_phy_setInterfaceMode(port, DYNAMIXEL_INTERFACE_MODE_RX);

		mode->rtxMode = PORT_MODE_IDLE;
		USART_CR1(interface->serialInterface) &= ~USART_CR1_TCIE;
		USART_CR3(interface->serialInterface) &= ~USART_CR3_DMAT;

		/* clear interrupt pendig flag of corresponding DMA */
		DMA_HIFCR(interface->dma) = (DMA_HISR_TCIF7) | (DMA_HISR_HTIF7) | (DMA_HISR_TEIF7) | (DMA_HISR_DMEIF7) | (DMA_HISR_TCIF7) | (DMA_HISR_FEIF7);

		/* tell the world! */
		if (NULL != g_txCompleteCallback)
		{
			(void)(g_txCompleteCallback)(port);
		}
	}
}

static void dynamixelPhyInitFunction(void);
MODULE_INIT_FUNCTION(dynamixelPhy, 4, dynamixelPhyInitFunction)
static void dynamixelPhyInitFunction(void)
{
	/* setup interfaces */
	uint8_t i = 0U;

	g_txCompleteCallback = NULL;
	g_receiveCallback = NULL;

	for (i = 0U; i < (sizeof(g_interfaces) / sizeof(rs485_interface_t)); ++i)
	{
		setupInterface(&(g_interfaces[i]));
		dynamixel_phy_setInterfaceMode(i, DYNAMIXEL_INTERFACE_MODE_RX);
	}

	for (i = 0U; i < DYNAMIXEL_PORT_CNT; ++i)
	{
		g_portModes[i].bytesToSend = 0U;
		g_portModes[i].rtxMode = PORT_MODE_IDLE;
		g_portModes[i].txBuf = NULL;
		g_portModes[i].bytesSent = 0U;
		dynamixel_phy_setInterfaceMode(i, DYNAMIXEL_INTERFACE_MODE_RX);
	}
}

