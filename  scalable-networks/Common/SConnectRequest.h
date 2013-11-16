/*
 * SConnectRequest.h
 *
 *  Created on: Nov 15, 2013
 *      Author: Niranjan
 */

#ifndef SCONNECTREQUEST_H_
#define SCONNECTREQUEST_H_

#include <SNodeInformation.h>
#include <EMessageId.h>

typedef struct
{
	eMessageId 		messageId;
	SNodeInformation nodeInformation;
}mConnectRequest;

#endif /* SCONNECTREQUEST_H_ */
