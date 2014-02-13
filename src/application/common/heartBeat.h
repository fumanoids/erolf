#ifndef APP_HEARTBEAT
#define APP_HEARTBEAT


/*
 * @param active: 0 - heartbeat off
 *                1 - heartbeat on
 */
void setHeartBeat(int active);


/*
 * @return: 0 - heartbeat off
 *          1 - heartbeat on
 */
int getHeartBeat(void);


/*
 * @param active: 0 - off
 *                1 - on
 */
void setHeartBeatLED(int active);

#endif
