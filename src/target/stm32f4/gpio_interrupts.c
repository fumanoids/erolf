/*
 * gpio_interrupts.c
 *
 *  Created on: 15.01.2014
 *      Author: lutz
 */

#include <flawless/stdtypes.h>
#include <flawless/init/systemInitializer.h>
#include <flawless/platform/system.h>

#include <libopencm3/stm32/f4/syscfg.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/nvic.h>
#include <libopencm3/stm32/f4/gpio.h>
#include <libopencm3/stm32/f4/rcc.h>

#include "gpio_interrupts.h"


typedef struct tag_gpioInterruptInfo
{
	gpio_interruptCallback callback;
	const void *info;
} gpioInterruptInfo_t;

static gpioInterruptInfo_t g_interruptHandles[GPIO_MAX_PIN];

void gpio_registerFor_interrupt(gpio_interruptCallback callback, gpioPort_t port, gpioPin_t pin, gpioTriggerLevel_t triggerLevel, const void *info)
{
	uint8_t portIdx = 0;
	uint8_t pinIdx = 0;

	portIdx = (port - GPIOA) / (GPIOB - GPIOA);

	for (pinIdx = 0; pinIdx < sizeof(gpioPort_t) * 8; ++pinIdx)
	{
		if (0 != (pin & (1 << pinIdx)))
		{
			/* make sure that we use only one pin */
			pin = (1 << pinIdx);
			break;
		}
	}

	if (portIdx < GPIO_MAX_PIN)
	{
		gpioInterruptInfo_t * handle = &(g_interruptHandles[pinIdx]);
		if (NULL == handle->callback || callback == handle->callback)
		{
			/* enable that port */
			RCC_AHB1ENR |= portIdx;
			gpio_mode_setup(port, GPIO_MODE_INPUT, GPIO_PUPD_NONE, pin);

			system_mutex_lock();
			handle->callback = callback;
			handle->info = info;
			system_mutex_unlock();

			/* connect the gpio to the exti line */
			const uint8_t syscfgExtiIndex    = pinIdx / 4;
			const uint8_t syscfgExtiSubIndex = (pinIdx % 4) * 4;
			volatile uint32_t *syscfgExtiBasePtr = &SYSCFG_EXTICR1;
			syscfgExtiBasePtr[syscfgExtiIndex] = ((syscfgExtiBasePtr[syscfgExtiIndex]) & ~(0xf << syscfgExtiSubIndex)) | (portIdx << syscfgExtiSubIndex);

			if (0 != (triggerLevel & GPIO_TRIGGER_LEVEL_RISING))
			{
				EXTI_RTSR |= pin;
			} else
			{
				EXTI_RTSR &= ~pin;
			}
			if (0 != (triggerLevel & GPIO_TRIGGER_LEVEL_FALLING))
			{
				EXTI_FTSR |= pin;
			} else
			{
				EXTI_FTSR &= ~pin;
			}

			/* enable that interrupt */
			uint8_t interrupt = NVIC_EXTI0_IRQ;
			switch (pinIdx) {
				case 0:
					interrupt = NVIC_EXTI0_IRQ;
					break;
				case 1:
					interrupt = NVIC_EXTI1_IRQ;
					break;
				case 2:
					interrupt = NVIC_EXTI2_IRQ;
					break;
				case 3:
					interrupt = NVIC_EXTI3_IRQ;
					break;
				case 4:
					interrupt = NVIC_EXTI4_IRQ;
					break;
				default:
					break;
			}
			if (pinIdx > 4 && pinIdx < 10)
			{
				interrupt = NVIC_EXTI9_5_IRQ;
			} else if (pinIdx >= 10 && pinIdx <= 15)
			{
				interrupt =  NVIC_EXTI15_10_IRQ;
			}
			nvic_enable_irq(interrupt);
		}
	}
}

void gpio_unregisterFor_interrupt(gpio_interruptCallback callback, gpioPort_t port, gpioPin_t pin)
{
	uint8_t pinIdx = 0;

	for (pinIdx = 0; pinIdx < sizeof(gpioPort_t) * 8; ++pinIdx)
	{
		if (0 != (pin & (1 << pinIdx)))
		{
			break;
		}
	}
	if (pinIdx < GPIO_MAX_PIN)
	{
		gpioInterruptInfo_t * handle = &(g_interruptHandles[pinIdx]);
		if (callback == handle->callback)
		{
			system_mutex_lock();
			handle->callback = NULL;
			handle->info = NULL;
			gpio_disable_interrupt(port, pin);
			EXTI_RTSR &= ~pin;
			EXTI_FTSR &= ~pin;
			system_mutex_unlock();
		}
	}
}

void gpio_enable_interrupt(gpioPort_t port, gpioPin_t pin)
{
	EXTI_PR = pin; /* clear interrupt pending bit */
	EXTI_IMR |= pin;
	UNUSED(port);
}

void gpio_disable_interrupt(gpioPort_t port, gpioPin_t pin)
{
	EXTI_IMR &= ~pin;
	EXTI_PR = pin; /* clear interrupt pending bit */
	UNUSED(port);
}

static void dispatchInterrupt(const uint8_t index)
{
	if (NULL != g_interruptHandles[index].callback)
	{
		(void)(g_interruptHandles[index].callback)((void*)g_interruptHandles[index].info);
	}
	EXTI_PR = (1 << index);
}

void exti0_isr(void)
{
	const uint8_t index = 0;
	dispatchInterrupt(index);
}

void exti1_isr(void)
{
	const uint8_t index = 1;
	dispatchInterrupt(index);
}

void exti2_isr(void)
{
	const uint8_t index = 2;
	dispatchInterrupt(index);
}

void exti3_isr(void)
{
	const uint8_t index = 3;
	dispatchInterrupt(index);
}

void exti4_isr(void)
{
	const uint8_t index = 4;
	dispatchInterrupt(index);
}

void exti9_5_isr(void)
{
	uint8_t i;
	for (i = 5; i < 10; ++i)
	{
		if (0 != (EXTI_PR & (1 << i)))
		{
			dispatchInterrupt(i);
		}
	}
}

void exti15_10_isr(void)
{
	uint8_t i;
	for (i = 10; i < 16; ++i)
	{
		if (0 != (EXTI_PR & (1 << i)))
		{
			dispatchInterrupt(i);
		}
	}
}

static void gpio_interrupts_init(void);
MODULE_INIT_FUNCTION(gpio_interrupts, 3, gpio_interrupts_init)
static void gpio_interrupts_init(void)
{
	RCC_APB2ENR |= RCC_APB2ENR_SYSCFGEN;

	uint8_t i;
	for (i = 0; i < GPIO_MAX_PIN; ++i)
	{
		g_interruptHandles[i].callback = NULL;
		g_interruptHandles[i].info = NULL;
	}
}
