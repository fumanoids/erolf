/*
 * gpio_interrupts.h
 *
 *  Created on: 15.01.2014
 *      Author: lutz
 */

#ifndef GPIO_INTERRUPTS_H_
#define GPIO_INTERRUPTS_H_



typedef void (*gpio_interruptCallback)(void *info);

typedef uint32_t gpioPort_t;

typedef uint32_t gpioPin_t;

#define GPIO_MAX_PIN 16U

typedef enum
{
	GPIO_TRIGGER_LEVEL_RISING = 1,
	GPIO_TRIGGER_LEVEL_FALLING = 2
}gpioTriggerLevel_t;


void gpio_registerFor_interrupt(gpio_interruptCallback callback, gpioPort_t port, gpioPin_t pin, gpioTriggerLevel_t triggerLevel, const void *info);
void gpio_unregisterFor_interrupt(gpio_interruptCallback callback, gpioPort_t port, gpioPin_t pin);
void gpio_enable_interrupt(gpioPort_t port, gpioPin_t pin);
void gpio_disable_interrupt(gpioPort_t port, gpioPin_t pin);


#endif /* GPIO_INTERRUPTS_H_ */
