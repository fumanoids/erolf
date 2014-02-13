/*
 * RPC_Interface.h
 *
 *  Created on: Apr 25, 2012
 *      Author: lutz
 */

#ifndef RPC_INTERFACE_H_
#define RPC_INTERFACE_H_

typedef int8_t RPC_ID_t;
typedef uint16_t RPC_paramLen_t;

/*
 * the position of the rpc id in the packets (in byte)
 */
#define RPC_PACKET_INDEX_OF_RPC_ID 0U


/*
 * the position of the parameter length in the packets (in byte)
 */
#define RPC_PACKET_INDEX_OF_PARAMETER_LEN sizeof(RPC_ID_t)

#define RPC_PACKET_INDEX_Of_FLAGS (RPC_PACKET_INDEX_OF_PARAMETER_LEN + sizeof(RPC_paramLen_t))

/*
 * the position of the parameter in the outgoing packets (in byte)
 */
#define RPC_REQ_PACKET_INDEX_OF_PARAMETER (RPC_PACKET_INDEX_Of_FLAGS + sizeof(RPC_flags_t))

/*
 * the position of the parameter in the incomming packets (in byte)
 */
#define RPC_RESP_PACKET_INDEX_OF_PARAMETER (sizeof(RPC_ID_t) + sizeof(RPC_paramLen_t))


#define RPC_SUB_PROTOCOL_IDENTIFIER 1U

#define RPC_PROBE_SMALL_IDENTIFIER -1
#define RPC_PROBE_DESCRIPTIVE_IDENTIFIER -2


/*
 * flag indicating that the call will generate a response
 * (return value is transmitted from motorboard to the IGEP)
 */
#define RPC_FLAGS_REQUIRES_RESPONSE 1U

#define RPC_HEADER_SIZE (sizeof(RPC_ID_t) + sizeof(RPC_paramLen_t) + sizeof(RPC_flags_t))

typedef uint8_t  RPC_flags_t;

/*
 * TODO: besseren Namen ausdenken
 */
typedef struct tag_RPCInfo
{
	/*
	 * the target id of the RPC
	 * the implemented functions can be determined by invoking the probe RPC
	 */
	RPC_ID_t targetID;

	/*
	 * the size in bytes of the the argument
	 */
	RPC_paramLen_t paramLen;

	/*
	 * the size in bytes of the the return value
	 */
	RPC_paramLen_t retSize;

	/*
	 * this determines if the response packet can be shorter than retSize
	 */
	bool responsePacketCanBeShorter;

	/*
	 * packet requires response (return value of RPC will be read from bus)
	 */
	RPC_flags_t flags;

	/*
	 * pointer to the buffer of the argument
	 */
	const void *argumentBuffer;

	/*
	 * pointer to a buffer for the return value
	 */
	void *retValBuffer;

} RPC_Info_t;

/*
 * a RPC packet starts with the subprotocolID 1 (1 byte)
 * then the targetRPC (index of RPC function) is transmitted (1 byte)
 * parameterLen (2 bytes, the parameter we send)
 * 1 byte (requires response + some flags which are not defined yet)
 * everything after that is the argument to the rpc function
 */

#endif /* RPC_INTERFACE_H_ */
