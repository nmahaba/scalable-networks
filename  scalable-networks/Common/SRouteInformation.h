/*
 * SRouteInformation.h
 *
 *  Created on: Nov 15, 2013
 *      Author: Niranjan
 */

#ifndef SROUTEINFORMATION_H_
#define SROUTEINFORMATION_H_

#include <EMessageId.h>
#include <vector>
#include <Constants.h>

typedef struct
{
	eMessageId 		messageId;
	int 			nodeId;
    int 			newDistV[MAX_NUMBER_OF_NODES] ;
    int 			newDegV[MAX_NUMBER_OF_NODES] ;
}mRouteInformation;

#endif /* SROUTEINFORMATION_H_ */
