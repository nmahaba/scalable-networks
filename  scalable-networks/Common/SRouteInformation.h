/*
 * SRouteInformation.h
 *
 *  Created on: Nov 15, 2013
 *      Author: Niranjan
 */

#ifndef SROUTEINFORMATION_H_
#define SROUTEINFORMATION_H_

#include <vector>

typedef struct
{
	eMessageId 		messageId;
	int 			nodeId;
	std::vector<int> degreeVector;
	std::vector<int> distanceVector;
}mRouteInformation;

#endif /* SROUTEINFORMATION_H_ */
