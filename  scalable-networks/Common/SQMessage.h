/*
 * SQMessage.h
 *
 *  Created on: Dec 4, 2013
 *      Author: Niranjan
 */

#ifndef SQMESSAGE_H_
#define SQMESSAGE_H_

#include <Constants.h>

typedef struct
{
	int nodeId;
	int fdistance;
	int routing_tab[MAX_NUMBER_OF_NODES][3];
}SQMessage;

#endif /* SQMESSAGE_H_ */
