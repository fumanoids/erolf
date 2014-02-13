/*
 * buzzer.c
 *
 *  Created on: Mar 14, 2013
 *      Author: lutz
 */

#include <flawless/init/systemInitializer.h>
#include <flawless/core/msg_msgPump.h>
#include <flawless/config/msgIDs.h>
#include <flawless/config/msgProxyConfig.h>

#include <flawless/timer/swTimer.h>
#include <interfaces/systemTime.h>
#include <interfaces/buzzer.h>

#include <flawless/protocol/genericFlawLessProtocolApplication.h>
#include <flawless/protocol/genericFlawLessProtocol_Data.h>


#include <string.h>

MSG_PUMP_DECLARE_MESSAGE_BUFFER_POOL(relativeBatVoltage, uint8_t, 2, MSG_ID_REL_BAT_VOLTAGE)
MSG_PUMP_DECLARE_MESSAGE_BUFFER_POOL(relativeAbsVoltage, uint16_t, 2, MSG_ID_BAT_VOLTAGE)

static void onTimer(void);
static void onTimer(void)
{
	static uint32_t freq = 50;
	setFreq1(freq, FREQ_SYNTH_PROFILE_SQUARE);
	setFreq2(5000 + freq % 5000, FREQ_SYNTH_PROFILE_SINE);

	freq = freq + freq / 3;
	if (freq > 20000)
	{
		freq = 50;
	}
}

static void onRelBatVoltage(msgPump_MsgID_t msgID, const void *data);
static void onRelBatVoltage(msgPump_MsgID_t msgID, const void *data)
{
	if (MSG_ID_REL_BAT_VOLTAGE == msgID)
	{
		const uint8_t *relVoltage = (const uint8_t*)data;
		static bool isRegistered = false;

		if (*relVoltage < 75) /* roughly 10% */
		{
			if (false == isRegistered)
			{
				swTimer_registerOnTimer(&onTimer, 100, false);
			}
			isRegistered = true;
		} else
		{
			if (false != isRegistered)
			{
				setFreq1(0, FREQ_SYNTH_PROFILE_SINE);
				swTimer_unRegisterFromTimer(&onTimer);
				isRegistered = false;
			}
		}
	}
}

static void onAbsBatVoltage(msgPump_MsgID_t msgID, const void *data);
static void onAbsBatVoltage(msgPump_MsgID_t msgID, const void *data)
{
	uint16_t absVolage = *((const uint16_t*)data);
	UNUSED(absVolage);
	UNUSED(msgID);
}

static void buzzer_init(void);
MODULE_INIT_FUNCTION(buzzer, 6, buzzer_init)
static void buzzer_init(void)
{
	msgPump_registerOnMessage(MSG_ID_REL_BAT_VOLTAGE, &onRelBatVoltage);
	msgPump_registerOnMessage(MSG_ID_BAT_VOLTAGE, &onAbsBatVoltage);

	playTheme(BUILT_IN_THEME_MARIO_THEME_SHORT);
//	swTimer_registerOnTimer(&onTimer, 100, false);
}

/** receives incoming messages from odroid
 */
static void beepSoundEndpoint(const void *ipacket, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor);
GENERIC_PROTOCOL_ENDPOINT(beepSoundEndpoint, 31);
static void beepSoundEndpoint(const void *ipacket, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor)
{
	UNUSED(i_interfaceDescriptor);
	UNUSED(i_packetLen);

	uint8_t value;
	memcpy(&value, ipacket, sizeof(uint8_t));

	if (value == 1) {
		playTheme(BUILT_IN_BEEP_ONCE);
	} else if (value == 2) {
		playTheme(BUILT_IN_BEEP_TWICE);
	} else if (value == 3) {
		playTheme(BUILT_IN_BEEP_TRIPPLE);
	}

}
