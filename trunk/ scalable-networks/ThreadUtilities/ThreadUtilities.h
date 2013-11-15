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

/*********************************************************************************************************
 /** spawnUdpThreadForQueries: Function to spawn a thread to handle UDP messages for node queries
  *
  *  Block on a UDP Socket for Query message and reply when a query arrives
  *
  * @return -1 if error
 *
 *********************************************************************************************************/
int spawnUdpThreadForQueries();

/*
 * handleFileTransferRequest: Function executes on a new thread and handles the UDP, node query messages
 */
void *handleFileTransferRequest(void *data);


#endif /* THREADUTILITIES_H_ */
