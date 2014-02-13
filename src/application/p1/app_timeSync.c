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

static uint64_t startTimeMeasurementUS = 0;
static uint8_t measurementCount = 0;
static int64_t lastPingUS = 2000000; // 2 Seconds start delay

static uint8_t syncTries = 0;

static void timeSyncEndpoint(const void *ipacket, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor);
GENERIC_PROTOCOL_ENDPOINT(timeSyncEndpoint, TIMESYNCCOMM_SUB_PROTOCOL_ID)
static void timeSyncEndpoint(const void *ipacket, uint16_t i_packetLen, flawLessInterfaceDescriptor_t i_interfaceDescriptor)
{
	/* if we have received a packet from interface 0 we have to push it out on interface 1 and vice versa */
	if (0 == i_interfaceDescriptor) // Package from P2
	{
		if(i_packetLen != sizeof(imuOutgoingMsg_t)) return; // Wrong package length

		imuOutgoingMsg_t const* dataIn = (imuOutgoingMsg_t const*) ipacket;
		imuOutgoingMsg_t dataOut;
		memcpy(&dataOut, dataIn, sizeof(dataOut));

		dataOut.p1BelieveTimeUS = getCurrentTimeUS();
		dataOut.p1PingUS = lastPingUS;
		startTimeMeasurementUS = getSystemTimeUS();

		++syncTries;
		if (syncTries >= 10) {
			syncTries = 10;
			timeSetSync(0);
		}

		genericProtocol_sendMessage(1, TIMESYNCCOMM_SUB_PROTOCOL_ID, sizeof(dataOut), &dataOut);

	} else if (1 == i_interfaceDescriptor) //Package from ordroid
	{
		if (i_packetLen != sizeof(imuIncomingMsg_t)) return; // Wrong package length
		imuIncomingMsg_t const* data = (imuIncomingMsg_t const*) ipacket;
		uint64_t endTimeMeasurementUS = getSystemTimeUS();
		genericProtocol_sendMessage(0, TIMESYNCCOMM_SUB_PROTOCOL_ID, i_packetLen, ipacket);
		uint64_t odroidBelieveTimeUS;
		memcpy(&odroidBelieveTimeUS, data, sizeof(odroidBelieveTimeUS));

		int64_t pingUS = (endTimeMeasurementUS - startTimeMeasurementUS) / 2ULL;
		++measurementCount;
//		if (timeIsSync() == 0 || lastPingUS * (100+measurementCount) / 100 > pingUS)
		{
			int64_t timeDiffUS = (int64_t)(odroidBelieveTimeUS) + pingUS - (int64_t)(endTimeMeasurementUS);
			setCurrentTimeDiffUS(timeDiffUS);
			measurementCount = 0;
			lastPingUS = pingUS;
		}

		timeSetSync(1);
		syncTries = 0;
	}
}


