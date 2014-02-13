/*
 * communication.h
 *
 *  Created on: Apr 28, 2012
 *      Author: lutz
 */

#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_


#define HTONS(a) ((((a) >> 8) & 0x00ff) | (((a) << 8) & 0xff00))
#define NTOHS(a) HTONS((a))

#define HTONL(a) ((((a) >> 24) & 0x00ff) | (((a) >> 8) & 0xff00) | (((a) << 8) & 0xff0000) | (((a) << 24) & 0xff000000))
#define NTOHL(a) HTONL(a)

#endif /* COMMUNICATION_H_ */
