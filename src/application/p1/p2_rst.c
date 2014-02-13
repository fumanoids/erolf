

#include <flawless/init/systemInitializer.h>

#include <libopencm3/stm32/f4/gpio.h>
#include <libopencm3/stm32/f4/rcc.h>

#define P2_RST_PORT GPIOA
#define P2_RST_PIN GPIO5

static void p2Rst_init(void);
MODULE_INIT_FUNCTION(p2Rst, 1, p2Rst_init);
static void p2Rst_init(void)
{
	RCC_AHB1ENR |= RCC_AHB1ENR_IOPAEN;
	gpio_mode_setup(P2_RST_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, P2_RST_PIN);
	gpio_set_output_options(P2_RST_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_100MHZ, P2_RST_PIN);
	gpio_set(P2_RST_PORT, P2_RST_PIN);
}
