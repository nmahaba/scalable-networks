/*
 * SJoinRequest.h
 *
 *  Created on: Nov 15, 2013
 *      Author: Niranjan
 */

#ifndef SJOINREQUEST_H_
#define SJOINREQUEST_H_

#include <EMessageId.h>
#include <Constants.h>

typedef struct
{
	eMessageId 		messageId;
	SNodeInformation nodeInformation;
}mJoinRequest;

#endif /* SJOINREQUEST_H_ */
