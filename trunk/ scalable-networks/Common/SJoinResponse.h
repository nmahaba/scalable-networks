/*
 * SJoinResponse.h
 *
 *  Created on: Nov 15, 2013
 *      Author: Niranjan
 */

#ifndef SJOINRESPONSE_H_
#define SJOINRESPONSE_H_

#include <EMessageId.h>
#include <Constants.h>
#include <SNodeInformation.h>

typedef struct
{
	eMessageId 		messageId;
	int				nodeCount;
	SNodeInformation nodeInformation[MAX_NUM_OF_NON_PRIME_NODES];
}mJoinResponse;

#endif /* SJOINRESPONSE_H_ */
