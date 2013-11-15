/*
 * ThreadUtilities.c
 *
 *  Created on: Nov 15, 2013
 *      Author: Niranjan
 */
#include <ThreadUtilities.h>
#include <pthread.h>

/* Extern */
extern pthread_attr_t udpQueryThreadAttr;
extern pthread_t udpQueryThread;
extern pthread_attr_t udpEntryHandlerThreadAttr;
extern pthread_t udpEntryHandlerThread;

/*********************************************************************************************************
 /** spawnUdpThreadForQueries: Function to spawn a thread to handle UDP messages for node queries
  *
  *  Block on a UDP Socket for Query message and reply when a query arrives
  *
  * @return -1 if error
 *
 *********************************************************************************************************/
int spawnUdpThreadForQueries()
{
	int status;

	pthread_attr_init(&udpQueryThreadAttr);

	pthread_attr_setdetachstate(&udpQueryThreadAttr, PTHREAD_CREATE_JOINABLE);

	if((status = pthread_create(&udpQueryThread, &udpQueryThreadAttr, handleFileNodeQueries, 0)) == -1)
	{
		printf("ERROR: spawnUdpThreadForQueries, failed spawning a thread, ECODE %s\n", strerror(errno));
		return -1;
	}

	return 0;
}

/*
 * handleFileNodeQueries: Function executes on a new thread and handles the UDP, node query messages
 */
void *handleFileNodeQueries(void *data)
{
	printf("INFO: UDP Thread for handling Node Query messages\n");

	while(1);
}

/*********************************************************************************************************
 /** spawnUdpThreadForEntryHandler: Function to spawn a thread to handle UDP messages for JoinReq, JoinResp
  *
  *  Block on a UDP Socket for JoinReq message, do the processing and send the JoinResp message
  *
  * @return -1 if error
 *
 *********************************************************************************************************/
int spawnUdpThreadForEntryHandler()
{
	int status;

	pthread_attr_init(&udpEntryHandlerThreadAttr);

	pthread_attr_setdetachstate(&udpEntryHandlerThreadAttr, PTHREAD_CREATE_JOINABLE);

	if((status = pthread_create(&udpEntryHandlerThread, &udpEntryHandlerThreadAttr, handleNodeEntry, 0)) == -1)
	{
		printf("ERROR: spawnUdpThreadForEntryHandler, failed spawning a thread, ECODE %s\n", strerror(errno));
		return -1;
	}

	return 0;
}

/*
 * handleNodeEntry: Function executes on a new thread and handles the UDP, node query messages
 */
void *handleNodeEntry(void *data)
{
	printf("INFO: UDP Thread for handling Node Entry\n");

	while(1);
}

