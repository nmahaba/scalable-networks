/*
 * Main.c
 *
 *  Created on: Nov 1, 2013
 *      Author: Niranjan
 */

#include <iostream>
#include <FileUtilities.h>
#include <SocketUtilities.h>
#include <ThreadUtilities.h>
#include <Constants.h>
#include <stdlib.h>
#include <pthread.h>

//#define DEBUG

using namespace std;

/* Global variables - Start */
SNodeInformation nodeInformation[MAX_NUMBER_OF_NODES];
int connectionInfo[MAX_NUMBER_OF_NODES] =	{0};
int primeNode 							= 	0;

/* Mutex */
pthread_mutex_t mutex_nodeDB;

/***************** Thread attributes ****************/
/* UDP NodeQuery thread */
pthread_attr_t udpQueryThreadAttr;
pthread_t udpQueryThread;

/* UDP JoinReq/JoinResp thread */
pthread_attr_t udpEntryHandlerThreadAttr;
pthread_t udpEntryHandlerThread;

/* Global variables - End   */

/* Debug functions */
void printNodeDB(int ownNodeId)
{
	int index = 0;

	printf("*****************************************\n");
	printf("NodeId\tHostName\tTCPPort\tUDPPort\n");
	printf("*****************************************\n");

	for(index=1 ; index<MAX_NUMBER_OF_NODES ; index++)
	{
		printf("%s %15s %10s %10s\n",
				nodeInformation[index].nodeId,
				nodeInformation[index].hostName,
				nodeInformation[index].tcpPortNumber,
				nodeInformation[index].udpPortNumber);
	}
}

/* Debug functions */
void printConnectionInfo(int ownNodeId)
{
	int index = 0;

	printf("*****************************************\n");
	printf("Connectivity information\n");
	printf("*****************************************\n");
	printf("Node %d is connected to\n", ownNodeId);

	for(index=1 ; index<MAX_NUMBER_OF_NODES ; index++)
	{
		if(connectionInfo[index] != 0)
		{
			printf("%d ",index);
		}
	}
	printf("\n");
}

/*
 * ResourceCleanUp - Function which is called in the end to cleanup all resources
 * This should be a handler for a SIGKILL signal as our application runs infinitely
 * And will be terminated by pressing CTRL-C
 * */


/* Command line arguments
 * nodes.out nodeId nodeInformationFile connectionInfoFile*/

/*
 * Main functionalities
 * 1. Read nodeInfo file and update node database
 * 2. Spawn a thread for handling UDP connections.
 * 3. First Mo nodes
 * 		a. Read connectionInfo file and update connection database.
 * 		b. Send TCP connections to other Mo nodes.
 * 4. Other non Mo nodes
 * 		a. Query information from other nodes.
 * 		b. Compute probability and send TCP connection request to m nodes
 * 		c. Update new network information.
 * 5. Handle new and existing TCP connections.
 */
int main(int argc, char **argv)
{
	char nodeInformationFile[MAX_CHARACTERS_IN_FILENAME];
	char connectionInfoFile[MAX_CHARACTERS_IN_FILENAME];
	int ownNodeId = -1;

	/* Check command line arguments */
	if(argc != 5)
	{
		printf("ERROR: Invalid command line arguments\nnodes.out <<NodeId>> <<PrimeNode>> <<nodeInfoFile>> <<connectionInfoFile>>\n");
		return -1;
	}

	/* Extract command line arguments */
	/* Store own node information */
	ownNodeId = atoi(argv[1]);
	primeNode = atoi(argv[2]);
	strcpy(nodeInformationFile, argv[3]);
	strcpy(connectionInfoFile, argv[4]);

	/* Read the connections file and fill the database */
	if(initializeNodeDB(nodeInformationFile) == -1)
	{
		return -1;
	}

#ifdef DEBUG
	/* Debug function */
	printNodeDB(ownNodeId);
#endif

	/* Read connections file */
	if(readConnectionsFile(connectionInfoFile, ownNodeId) == -1)
	{
		return -1;
	}

#ifdef DEBUG
	/* Debug function */
	printConnectionInfo(ownNodeId);
#endif

	/* Initialize Mutex and Semaphores */
	pthread_mutex_init(&mutex_nodeDB, NULL);

	/* Establish connection with the prime nodes */
	if(connectToPrimeNodes(ownNodeId) == -1)
	{
		return -1;
	}

	/* Spawn the thread for handling UDP Statistics Queries */
	if(spawnUdpThreadForQueries(&ownNodeId) == -1)
	{
		return -1;
	}

	/* Spawn the thread for handling Join Request/Response messages */
	if(spawnUdpThreadForEntryHandler(&ownNodeId) == -1)
	{
		return -1;
	}

	/* Infinitely wait for new TCP connections or respond to other queries on TCP connections */
	processTCPConnections(ownNodeId);

	/* Join to one of the threads so that main doesn't exit killing all the processes */
	//pthread_join(udpQueryThread, NULL);

	/* Cleanup all the resources
	 * 1. Close all open sockets.
	 * 2. Destroy thread attributes.
	 * Write a function for it */

	return 0;
}





