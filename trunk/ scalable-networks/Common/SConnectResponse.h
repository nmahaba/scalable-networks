/*
 * SConnectResponse.h
 *
 *  Created on: Nov 15, 2013
 *      Author: Niranjan
 */

#ifndef SCONNECTRESPONSE_H_
#define SCONNECTRESPONSE_H_

#include <SNodeInformation.h>
#include <EMessageId.h>
#include <SRouteInformation.h>

typedef struct
{
	eMessageId 		messageId;
	int				nodeId;
	mRouteInformation routeInformation;
}mConnectResponse;

#endif /* SCONNECTRESPONSE_H_ */
