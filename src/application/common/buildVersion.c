/*
 * buildVersion.c
 *
 *  Created on: Mar 25, 2013
 *      Author: lutz
 */


#include <flawless/rpc/RPC.h>
#include <string.h>

#include "../../build_info.h"

typedef uint8_t buildInfoStr[sizeof(BUILDINFO)];
static const char buildVersion[] = BUILDINFO;

RPC_FUNCTION(getBuildVersion, "getBuildVersion", "that is pretty self explainig", uint8_t, buildInfoStr, true, true)
	memcpy(o_param, &buildVersion, sizeof(buildVersion));
}



