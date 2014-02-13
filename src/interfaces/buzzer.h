#ifndef BUZZER_H
#define BUZZER_H


typedef enum tag_freqSynthProfile
{
	FREQ_SYNTH_PROFILE_SINE = 0,
	FREQ_SYNTH_PROFILE_TRIANGLE = 1,
	FREQ_SYNTH_PROFILE_SQUARE = 2,
	FREQ_SYNTH_PROFILE_CNT = 3
} freqSynthProfile_t;

typedef enum tag_builtInSouds
{
	BUILT_IN_THEME_MARIO_THEME,
	BUILT_IN_THEME_MARIO_THEME_SHORT,
	BUILT_IN_BEEP_ONCE,
	BUILT_IN_BEEP_TWICE,
	BUILT_IN_BEEP_TRIPPLE
} builtIntTheme_t;

void setFreq(uint16_t freq, freqSynthProfile_t profile);

void setFreq1(uint16_t freq, freqSynthProfile_t profile);
void setFreq2(uint16_t freq, freqSynthProfile_t profile);


void playTheme(builtIntTheme_t sound);


#endif
