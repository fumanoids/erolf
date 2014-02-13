/*
 * dac.c
 *
 *  Created on: 26.11.2013
 *      Author: lutz
 */

#include <libopencm3/stm32/dac.h>
#include <libopencm3/stm32/f4/rcc.h>
#include <libopencm3/stm32/f4/gpio.h>
#include <libopencm3/stm32/f4/timer.h>
#include <libopencm3/stm32/f4/dma.h>

#include <flawless/init/systemInitializer.h>
#include <flawless/timer/swTimer.h>

#include <interfaces/systemTime.h>
#include <interfaces/buzzer.h>
#include <math.h>
#include <target/stm32f4/clock.h>

#define PI 3.1415926536f
#define BUFFERSIZE 32

#define DAC_MAX_VAL ((1 << 11) - 1)

static uint16_t g_freqSynth_data[FREQ_SYNTH_PROFILE_CNT][BUFFERSIZE];

#define DAC_HW_TIMER1 TIM3
#define DAC_HW_TIMER2 TIM4

#define DAC_DMA1 DMA1
#define DAC_DMA1_STREAM DMA_STREAM_2
#define DAC_DMA2 DMA1
#define DAC_DMA2_STREAM DMA_STREAM_6

typedef struct tag_note
{
	uint16_t freq;
	uint32_t durationMS;
	uint32_t pauseAfterMS;
} note_t;

static const note_t g_beepOnce[] =
{
	{500, 100, 150}
};

static const note_t g_beepTwice[] =
{
	{500, 100, 150},
	{600, 100, 150}
};

static const note_t g_beepTripple[] =
{
	{500, 100, 150},
	{600, 100, 150},
	{700, 100, 150}

};


static const note_t g_marioTheme[] =
{
	{660, 100, 150},
	{660, 100, 300},
	{660, 100, 300},
	{510, 100, 100},
	{660, 100, 300},
	{770, 100, 550},
	{380, 100, 575},

	{510, 100, 450},
	{380, 100, 400},
	{320, 100, 500},
	{440, 100, 300},
	{480, 80, 330},
	{450, 100, 150},
	{430, 100, 300},
	{380, 100, 200},
	{660, 80, 200},
	{760, 50, 150},
	{860, 100, 300},
	{700, 80, 150},
	{760, 50, 350},
	{660, 80, 300},
	{520, 80, 150},
	{580, 80, 150},
	{480, 80, 500},

	{510, 100, 450},
	{380, 100, 400},
	{320, 100, 500},
	{440, 100, 300},
	{480, 80, 330},
	{450, 100, 150},
	{430, 100, 300},
	{380, 100, 200},
	{660, 80, 200},
	{760, 50, 150},
	{860, 100, 300},
	{700, 80, 150},
	{760, 50, 350},
	{660, 80, 300},
	{520, 80, 150},
	{580, 80, 150},
	{480, 80, 500},

	{500, 100, 300},

	{760, 100, 100},
	{720, 100, 150},
	{680, 100, 150},
	{620, 150, 300},

	{650, 150, 300},
	{380, 100, 150},
	{430, 100, 150},

	{500, 100, 300},
	{430, 100, 150},
	{500, 100, 100},
	{570, 100, 220},

	{500, 100, 300},

	{760, 100, 100},
	{720, 100, 150},
	{680, 100, 150},
	{620, 150, 300},

	{650, 200, 300},

	{1020, 80, 300},
	{1020, 80, 150},
	{1020, 80, 300},

	{380, 100, 300},
	{500, 100, 300},

	{760, 100, 100},
	{720, 100, 150},
	{680, 100, 150},
	{620, 150, 300},

	{650, 150, 300},
	{380, 100, 150},
	{430, 100, 150},

	{500, 100, 300},
	{430, 100, 150},
	{500, 100, 100},
	{570, 100, 420},

	{585, 100, 450},

	{550, 100, 420},

	{500, 100, 360},

	{380, 100, 300},
	{500, 100, 300},
	{500, 100, 150},
	{500, 100, 300},

	{500, 100, 300},

	{760, 100, 100},
	{720, 100, 150},
	{680, 100, 150},
	{620, 150, 300},

	{650, 150, 300},
	{380, 100, 150},
	{430, 100, 150},

	{500, 100, 300},
	{430, 100, 150},
	{500, 100, 100},
	{570, 100, 220},

	{500, 100, 300},

	{760, 100, 100},
	{720, 100, 150},
	{680, 100, 150},
	{620, 150, 300},

	{650, 200, 300},

	{1020, 80, 300},
	{1020, 80, 150},
	{1020, 80, 300},

	{380, 100, 300},
	{500, 100, 300},

	{760, 100, 100},
	{720, 100, 150},
	{680, 100, 150},
	{620, 150, 300},

	{650, 150, 300},
	{380, 100, 150},
	{430, 100, 150},

	{500, 100, 300},
	{430, 100, 150},
	{500, 100, 100},
	{570, 100, 420},

	{585, 100, 450},

	{550, 100, 420},

	{500, 100, 360},

	{380, 100, 300},
	{500, 100, 300},
	{500, 100, 150},
	{500, 100, 300},

	{500, 60, 150},
	{500, 80, 300},
	{500, 60, 350},
	{500, 80, 150},
	{580, 80, 350},
	{660, 80, 150},
	{500, 80, 300},
	{430, 80, 150},
	{380, 80, 600},

	{500, 60, 150},
	{500, 80, 300},
	{500, 60, 350},
	{500, 80, 150},
	{580, 80, 150},
	{660, 80, 550},

	{870, 80, 325},
	{760, 80, 600},

	{500, 60, 150},
	{500, 80, 300},
	{500, 60, 350},
	{500, 80, 150},
	{580, 80, 350},
	{660, 80, 150},
	{500, 80, 300},
	{430, 80, 150},
	{380, 80, 600},

	{660, 100, 150},
	{660, 100, 300},
	{660, 100, 300},
	{510, 100, 100},
	{660, 100, 300},
	{770, 100, 550},
	{380, 100, 575}
};



static const note_t g_startSound[] =
{
	{660, 100, 150},
	{660, 100, 300},
	{660, 100, 300},
	{510, 100, 100},
	{660, 100, 300},
	{770, 100, 550},
	{380, 100, 575}
};


static uint16_t g_themeIndex = 0;
static uint16_t g_musicNotesCnt = 0;
static const note_t *g_musicPtr = NULL;

static void playNextNote(void);
static void onTimerAfterDuration(void);

static void onTimerAfterDuration(void)
{
	setFreq(0, FREQ_SYNTH_PROFILE_SQUARE);
	const note_t * curNote = &(g_musicPtr[g_themeIndex]);
	const timerInterval_t pause = (curNote->pauseAfterMS - curNote->durationMS <= 0)? 0 : curNote->pauseAfterMS - curNote->durationMS;
	++g_themeIndex;
	swTimer_registerOnTimer(&playNextNote, pause, true);
}

static void playNextNote(void)
{
	if (g_themeIndex >= g_musicNotesCnt)
	{
		g_themeIndex = 0;
		setFreq(0, FREQ_SYNTH_PROFILE_SQUARE);
		return;
	}
	const note_t * curNote = &(g_musicPtr[g_themeIndex]);
	setFreq(curNote->freq, FREQ_SYNTH_PROFILE_SQUARE);
	swTimer_registerOnTimer(&onTimerAfterDuration, curNote->durationMS, true);
}

static void playTheme_internal(const note_t *theme, uint16_t noteCnt)
{
	g_themeIndex = 0;
	g_musicPtr = theme;
	g_musicNotesCnt = noteCnt;
	playNextNote();
}

void playTheme(builtIntTheme_t sound)
{
	switch (sound) {
		case BUILT_IN_THEME_MARIO_THEME:
			playTheme_internal(g_marioTheme, sizeof(g_marioTheme) / sizeof(g_marioTheme[0]));
			break;

		case BUILT_IN_THEME_MARIO_THEME_SHORT:
			playTheme_internal(g_startSound, sizeof(g_startSound) / sizeof(g_startSound[0]));
			break;
		case BUILT_IN_BEEP_ONCE:
			playTheme_internal(g_beepOnce, sizeof(g_beepOnce) / sizeof(g_beepOnce[0]));
			break;
		case BUILT_IN_BEEP_TWICE:
			playTheme_internal(g_beepTwice, sizeof(g_beepTwice) / sizeof(g_beepTwice[0]));
			break;
		case BUILT_IN_BEEP_TRIPPLE:
			playTheme_internal(g_beepTripple, sizeof(g_beepTripple) / sizeof(g_beepTripple[0]));
			break;

		default:
			break;
	}
}

void setFreq1(uint16_t freq, freqSynthProfile_t profile)
{
	UNUSED(profile);
	const uint32_t timer = DAC_HW_TIMER1;
	TIM_CR1(timer) &= ~TIM_CR1_CEN;
	while (0 != (DMA_SCCR(DAC_DMA1, DAC_DMA1_STREAM) & DMA_CR_EN))
	{
		DMA_SCCR(DAC_DMA1, DAC_DMA1_STREAM) &= ~DMA_CR_EN;
	}

	if (freq > 0)
	{
		uint32_t timeVal = CLOCK_APB1_TIMER_CLK / BUFFERSIZE / freq;
		const uint16_t psc = ((timeVal & 0xffff0000) >> 16U);
		const uint16_t arr = (timeVal / (psc + 1U));

		TIM_ARR(timer) = arr;
		TIM_PSC(timer) = psc;
		DMA_SM0AR(DAC_DMA1, DAC_DMA1_STREAM) = (uint32_t)g_freqSynth_data[profile];

		(void)TIM_CNT(timer);

		TIM_EGR(timer) |= TIM_EGR_UG;
		TIM_CR1(timer) |= TIM_CR1_CEN;
		DMA_SCCR(DAC_DMA1, DAC_DMA1_STREAM) |= DMA_CR_EN;
	} else
	{
		TIM_PSC(timer) = 0xffff;
		TIM_ARR(timer) = 0;
		TIM_CCR2(timer) = 0;

		/* generate update event, so that preload registers are transfered to shadow registers */
		TIM_EGR(timer) |= TIM_EGR_UG;
		DAC_DHR12R1 = 0;
	}
}

void setFreq2(uint16_t freq, freqSynthProfile_t profile)
{
	UNUSED(profile);
	const uint32_t timer = DAC_HW_TIMER2;
	TIM_CR1(timer) &= ~TIM_CR1_CEN;
	while (0 != (DMA_SCCR(DAC_DMA2, DAC_DMA2_STREAM) & DMA_CR_EN))
	{
		DMA_SCCR(DAC_DMA2, DAC_DMA2_STREAM) &= ~DMA_CR_EN;
	}

	if (freq > 0)
	{
		uint32_t timeVal = CLOCK_APB1_TIMER_CLK / BUFFERSIZE / freq;
		const uint16_t psc = ((timeVal & 0xffff0000) >> 16U);
		const uint16_t arr = (timeVal / (psc + 1U));

		TIM_ARR(timer) = arr;
		TIM_PSC(timer) = psc;
		DMA_SM0AR(DAC_DMA2, DAC_DMA2_STREAM) = (uint32_t)g_freqSynth_data[profile];

		(void)TIM_CNT(timer);

		TIM_EGR(timer) |= TIM_EGR_UG;
		TIM_CR1(timer) |= TIM_CR1_CEN;
		DMA_SCCR(DAC_DMA2, DAC_DMA2_STREAM) |= DMA_CR_EN;
	} else
	{
		TIM_PSC(timer) = 0xffff;
		TIM_ARR(timer) = 0;
		TIM_CCR2(timer) = 0;

		/* generate update event, so that preload registers are transfered to shadow registers */
		TIM_EGR(timer) |= TIM_EGR_UG;
		DAC_DHR12R2 = 0;
	}
}

void setFreq(uint16_t freq, freqSynthProfile_t profile)
{
	setFreq1(freq, profile);
	setFreq2(freq * 2, profile);
}

static void dac_init(void);
MODULE_INIT_FUNCTION(dac, 6, dac_init)
static void dac_init(void)
{
	uint16_t i = 0;

	RCC_APB1ENR |= RCC_APB1ENR_DACEN;
	RCC_AHB1ENR |= RCC_AHB1ENR_IOPAEN;
	RCC_AHB1ENR |= RCC_AHB1ENR_DMA1EN;
	RCC_APB1ENR |= RCC_APB1ENR_TIM3EN;
	RCC_APB1ENR |= RCC_APB1ENR_TIM4EN;

	float phase = 0.f;
	for (i = 0; i < BUFFERSIZE; ++i)
	{
		g_freqSynth_data[FREQ_SYNTH_PROFILE_SINE][i] = (uint16_t)((float)DAC_MAX_VAL * (sin(phase) + 1.f));
		g_freqSynth_data[FREQ_SYNTH_PROFILE_TRIANGLE][i] = (uint16_t)((float)DAC_MAX_VAL * ((float)i / (float)BUFFERSIZE));
		g_freqSynth_data[FREQ_SYNTH_PROFILE_SQUARE][i] = (i < BUFFERSIZE / 2) ? 0 : DAC_MAX_VAL;
		phase += 2. * PI / (float)BUFFERSIZE;
	}

	DMA_SCCR(DAC_DMA1, DAC_DMA1_STREAM) = 0;
	DMA_SCCR(DAC_DMA1, DAC_DMA1_STREAM) = (5 << 25) | (DMA_CR_PL_HIGH) | DMA_CR_MSIZE_HALFWORD | DMA_CR_PSIZE_HALFWORD | (DMA_CR_MINC) | (DMA_CR_CIRC)  | (1 << DMA_CR_DIR_SHIFT);
	DMA_SNDTR(DAC_DMA1, DAC_DMA1_STREAM) = BUFFERSIZE;
	DMA_SPAR(DAC_DMA1, DAC_DMA1_STREAM) = (uint32_t)&(DAC_DHR12R1);
	DMA_SM0AR(DAC_DMA1, DAC_DMA1_STREAM) = (uint32_t)g_freqSynth_data[FREQ_SYNTH_PROFILE_SINE];


	DMA_SCCR(DAC_DMA2, DAC_DMA2_STREAM) = 0;
	DMA_SCCR(DAC_DMA2, DAC_DMA2_STREAM) = (2 << 25) | (DMA_CR_PL_HIGH) | DMA_CR_MSIZE_HALFWORD | DMA_CR_PSIZE_HALFWORD | (DMA_CR_MINC) | (DMA_CR_CIRC)  | (1 << DMA_CR_DIR_SHIFT);
	DMA_SNDTR(DAC_DMA2, DAC_DMA2_STREAM) = BUFFERSIZE;
	DMA_SPAR(DAC_DMA2, DAC_DMA2_STREAM) = (uint32_t)&(DAC_DHR12R2);
	DMA_SM0AR(DAC_DMA2, DAC_DMA2_STREAM) = (uint32_t)g_freqSynth_data[FREQ_SYNTH_PROFILE_SINE];

	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO4);
	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO5);
	DAC_CR |= DAC_CR_EN1 | DAC_CR_EN2;


	/* setup timer */

	TIM_CR1(DAC_HW_TIMER1) |= TIM_CR1_ARPE;
	TIM_DIER(DAC_HW_TIMER1) |= TIM_DIER_UDE;

	TIM_CR1(DAC_HW_TIMER2) |= TIM_CR1_ARPE;
	TIM_DIER(DAC_HW_TIMER2) |= TIM_DIER_UDE;
}
