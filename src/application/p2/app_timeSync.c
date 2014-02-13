#include <interfaces/systemTime.h>

#include <flawless/stdtypes.h>
#include <flawless/core/msg_msgPump.h>
#include <flawless/config/msgIDs.h>

#include <flawless/init/systemInitializer.h>
#include <flawless/timer/swTimer.h>
#include <flawless/protocol/genericFlawLessProtocolApplication.h>
#include <flawless/protocol/genericFlawLessProtocol_Data.h>
#include <flawless/platform/system.h>
#include <string.h>

#define TIMESYNCCOMM_SUB_PROTOCOL_ID 12U

typedef struct _imuOutgoingMsg_t {
	uint64_t p1BelieveTimeUS;
	int64_t p1PingUS;
	uint64_t p2BelieveTimeUS;
	int64_t p2PingUS;
} imuOutgoingMsg_t;

typedef struct _imuIncomingMsg_t {
	uint64_t odroidBelieveTimeUS;
} imuIncomingMsg_t;

MSG_PUMP_DECLARE_MESSAGE_BUFFER_POOL(triggerTimeSyncBP, int, 2, MSG_ID_TRIGGER_TIME_SYNC)

static uint64_t startTimeMeasurementUS = 0;
static uint8_t measurementCount = 0;
static int64_t lastPingUS = 2000000; // 2 Seconds start delay

static uint8_t syncTries = 0;

static void onTriggerTimeSync(msgPump_MsgID_t msgID, const void *data);
static void onTriggerTimeSync(msgPump_MsgID_t msgID, const void *data)
{
	UNUSED(msgID);
	UNUSED(data);
	imuOutgoingMsg_t trans;
	trans.p2BelieveTimeUS = getCurrentTimeUS();
	trans.p2PingUS = lastPingUS;
	startTimeMeasurementUS = getSystemTimeUS();

	++syncTries;
	if (syncTries >= 10) {
		syncTries = 10;
		timeSetSync(0);
	}
	genericProtocol_sendMessage(0, TIMESYNCCOMM_SUB_PROTOCOL_ID, sizeof(trans), &trans);
}

static void timeSync_timerCB(void);
static void timeSync_timerCB(void)
{
	int *bla = NULL;
	msgPump_postMessage(MSG_ID_TRIGGER_TIME_SYNC, bla);
}

static void timeSync_init(void);
MODULE_INIT_FUNCTION(timesync, 9, timeSync_init);
static void timeSync_init(void)
{
	swTimer_registerOnTimer(&timeSync_timerCB, 100, false);
	msgPump_registerOnMessage(MSG_ID_TRIGGER_TIME_SYNC, &onTriggerTimeSync);
}

static void timeSyncEndpoint(const void *ipacket, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor);
GENERIC_PROTOCOL_ENDPOINT(timeSyncEndpoint, TIMESYNCCOMM_SUB_PROTOCOL_ID)
static void timeSyncEndpoint(const void *ipacket, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor)
{
	UNUSED(i_interfaceDescriptor);

	if (i_packetLen != sizeof(imuIncomingMsg_t)) return; // Wrong package size

	imuIncomingMsg_t const* data = (imuIncomingMsg_t const*) ipacket;
	uint64_t endTimeMeasurementUS = getSystemTimeUS();
	uint64_t odroidBelieveTimeUS;
	memcpy(&odroidBelieveTimeUS, data, sizeof(odroidBelieveTimeUS));

	int64_t pingUS = (endTimeMeasurementUS - startTimeMeasurementUS) / 2LL;
	++measurementCount;
//	if (timeIsSync() == 0 || lastPingUS * (100+measurementCount) / 100 > pingUS)
	{
		int64_t timeDiffUS = (int64_t)(odroidBelieveTimeUS) + pingUS - (int64_t)(endTimeMeasurementUS);
		setCurrentTimeDiffUS(timeDiffUS);
		measurementCount = 0;
		lastPingUS = pingUS;
	}

	timeSetSync(1);
	syncTries = 0;
}
