/*
 * ThreadUtilities.h
 *
 *  Created on: Nov 15, 2013
 *      Author: Niranjan
 */

#ifndef THREADUTILITIES_H_
#define THREADUTILITIES_H_

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <SocketUtilities.h>

/*********************************************************************************************************
 /** spawnUdpThreadForQueries: Function to spawn a thread to handle UDP messages for node queries
  *
  *  @param[in] ownNodeId: NodeId of the current node
  *
  *  Block on a UDP Socket for Query message and reply when a query arrives
  *
  * @return -1 if error
 *
 *********************************************************************************************************/
int spawnUdpThreadForQueries(int *ownNodeId);

/*
 * handleNodeQueries: Function executes on a new thread and handles the UDP, node query messages
 */
void *handleNodeQueries(void *data);

/*************************************************************************
 /** getRandomNumber : Function to return a random number in a given range
  *
  * param[in]	min Minimum bound
  *
  * param[in]	max Maximum bound
 *
 *  @return Random number within a range
 *
 ************************************************************************/
int getRandomNumber(int min, int max);


/*********************************************************************************************************
 /** spawnUdpThreadForJoinReqResp: Function to spawn a thread to handle UDP messages for JoinReq, JoinResp
  *
  *  Block on a UDP Socket for JoinReq message, do the processing and send the JoinResp message
  *
  * @return -1 if error
 *
 *********************************************************************************************************/
int spawnUdpThreadForEntryHandler(int *ownNodeId);

/*
 * handleNodeEntry: Function executes on a new thread and handles the UDP, node query messages
 */
void *handleNodeEntry(void *data);


#endif /* THREADUTILITIES_H_ */
