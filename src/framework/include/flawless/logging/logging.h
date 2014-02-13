/*
 * logging.h
 *
 *  Created on: 11.01.2012
 *      Author: lutz
 */

#ifndef LOGGING_H_
#define LOGGING_H_

#include <flawless/stdtypes.h>
#include <flawless/config/logging_config.h>

/* A handy wrapper for several loglevels
 *
 * You can specify the loglevel per file by defining LOG_LEVEL before including this header
 *
 * Loglevels are:
 * 0 - logoutput on error
 * 1 - logoutput on warning
 * 2 - info logoutput
 * 3 - verbose logoutput
 * 4 - misc logoutput
 *
 */

/* this is what will be transmitted instead of a log string. The index of the beginning in an array of all logstrings
 * hard to explain but basically something really easy
 */
typedef uint16_t logStringIndex_t;

enum tag_logLevel
{
	LOG_LEVEL_ERROR   = 0,
	LOG_LEVEL_WARNING = 1,
	LOG_LEVEL_INFO    = 2,
	LOG_LEVEL_VERBOSE = 3,
	LOG_LEVEL_MISC    = 4,
	LOG_LEVEL_CNT
};
typedef uint8_t logLevel_t;

#define LOGGING_CTL_SUB_PROTOCOL_ID 2
#define LOGGING_OUT_SUB_PROTOCOL_ID 3

#ifdef NO_LOG_OUTPUT

#define LOG_ERROR_0(string)
#define LOG_ERROR_1(string, arg1)
#define LOG_ERROR_2(string, arg1, arg2)

#define LOG_WARNING_0(string)
#define LOG_WARNING_1(string, arg1)
#define LOG_WARNING_2(string, arg1, arg2)

#define LOG_INFO_0(string)
#define LOG_INFO_1(string, arg1)
#define LOG_INFO_2(string, arg1, arg2)

#define LOG_VERBOSE_0(string)
#define LOG_VERBOSE_1(string, arg1)
#define LOG_VERBOSE_2(string, arg1, arg2)

#define LOG_MISC_0(string)
#define LOG_MISC_1(string, arg1)
#define LOG_MISC_2(string, arg1, arg2)

#else

/*
 * a handy logging function which we use internally.
 * Dont call that function directly!
 */
void logFunction0(const logLevel_t, const char *);
void logFunction1(const logLevel_t, const char *, int);
void logFunction2(const logLevel_t, const char *, int, int);

#define LOG_HELPER0(level, str) \
	{\
		__attribute__((section(".logStrings"))) \
		static const char logString[] = __FILE__ " - " str;\
		logFunction0(level, logString);\
	}

#define LOG_HELPER1(level, str, arg1) \
	{\
		__attribute__((section(".logStrings"))) \
		static const char logString[] = __FILE__ " - " str;\
		logFunction1(level, logString, arg1);\
	}

#define LOG_HELPER2(level, str, arg1, arg2) \
	{\
		__attribute__((section(".logStrings"))) \
		static const char logString[] = __FILE__ " - " str;\
		logFunction2(level, logString, arg1, arg2);\
	}

#define LOG_ERROR_0(string) LOG_HELPER0(LOG_LEVEL_ERROR, string)
#define LOG_ERROR_1(string, arg1) LOG_HELPER1(LOG_LEVEL_ERROR, string, arg1)
#define LOG_ERROR_2(string, arg1, arg2) LOG_HELPER2(LOG_LEVEL_ERROR, string, arg1, arg2)

#define LOG_WARNING_0(string) LOG_HELPER0(LOG_LEVEL_WARNING, string)
#define LOG_WARNING_1(string, arg1) LOG_HELPER1(LOG_LEVEL_WARNING, string, arg1)
#define LOG_WARNING_2(string, arg1, arg2) LOG_HELPER2(LOG_LEVEL_WARNING, string, arg1, arg2)

#define LOG_INFO_0(string) LOG_HELPER0(LOG_LEVEL_INFO, string)
#define LOG_INFO_1(string, arg1) LOG_HELPER1(LOG_LEVEL_INFO, string, arg1)
#define LOG_INFO_2(string, arg1, arg2) LOG_HELPER2(LOG_LEVEL_INFO, string, arg1, arg2)

#define LOG_VERBOSE_0(string) LOG_HELPER0(LOG_LEVEL_VERBOSE, string)
#define LOG_VERBOSE_1(string, arg1) LOG_HELPER1(LOG_LEVEL_VERBOSE, string, arg1)
#define LOG_VERBOSE_2(string, arg1, arg2) LOG_HELPER2(LOG_LEVEL_VERBOSE, string, arg1, arg2)

#define LOG_MISC_0(string) LOG_HELPER0(LOG_LEVEL_MISC, string)
#define LOG_MISC_1(string, arg1) LOG_HELPER1(LOG_LEVEL_MISC, string, arg1)
#define LOG_MISC_2(string, arg1, arg2) LOG_HELPER2(LOG_LEVEL_MISC, string, arg1, arg2)

#endif /* NO_LOG_OUTPUT */



#endif /* LOGGING_H_ */
