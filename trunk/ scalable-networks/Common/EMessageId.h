/*
 * EMessageId.h
 *
 *  Created on: Nov 15, 2013
 *      Author: Niranjan
 */

#ifndef EMESSAGEID_H_
#define EMESSAGEID_H_

typedef enum
{
	JoinRequest=1,
	JoinResponse,
	ConnectionRequest,
	ConnectionResponse,
	RouteInformation,
	InvalidMessageId
}eMessageId;

#endif /* EMESSAGEID_H_ */
