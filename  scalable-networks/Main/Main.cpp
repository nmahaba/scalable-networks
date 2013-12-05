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
#include <semaphore.h>
#include <signal.h>
#include <vector>

//#define DEBUG

using namespace std;

/* Global variables - Start */
SNodeInformation nodeInformation[MAX_NUMBER_OF_NODES];
int connectionInfo[MAX_NUMBER_OF_NODES] =	{0};
int primeNode 							= 	0;
int	numOfPrimeNodes						=	0;
int numOfNodesToJoin					=	0;

/* Data structures required for the algorithm */
int DVM[MAX_NUMBER_OF_NODES][MAX_NUMBER_OF_NODES]; 	/* The distance vector matrix */
int DistV[MAX_NUMBER_OF_NODES];					 	/* The distance vector */
int DegV[MAX_NUMBER_OF_NODES];						/* The degree vector */
vector< vector<int> > AdjList(MAX_NUMBER_OF_NODES); /* Used initially for calculating the DegV */
vector<int> barbasiBag;								/* Used for Barbasi model */

/* Mutex */
pthread_mutex_t mutex_nodeDB;

/* Semaphore */
/* For waiting until all ConnectResponse messages are received */
sem_t sem_connectRespWait;
sem_t sem_startTcpConListen;

/***************** Thread attributes ****************/
/* UDP NodeQuery thread */
pthread_attr_t udpQueryThreadAttr;
pthread_t udpQueryThread;

/* UDP JoinReq/JoinResp thread */
pthread_attr_t udpEntryHandlerThreadAttr;
pthread_t udpEntryHandlerThread;

/* Global variables - End   */

/* Debug functions */
static void printNodeDB()
{
	int index = 0;

	/* Get lock for NodeDB */
	pthread_mutex_lock(&mutex_nodeDB);

	printf("*****************************************\n");
	printf("NodeId\tHostName\tTCPPort\tUDPPort\tTCPPort\n");
	printf("*****************************************\n");

	for(index=1 ; index<MAX_NUMBER_OF_NODES ; index++)
	{
		printf("%d %15s %10s %10s %d\n",
				nodeInformation[index].nodeId,
				nodeInformation[index].hostName,
				nodeInformation[index].tcpPortNumber,
				nodeInformation[index].udpPortNumber,
				nodeInformation[index].tcpSocketFd);
	}

	/* Release lock for NodeDB */
	pthread_mutex_unlock(&mutex_nodeDB);
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

void sigPipeHandler(int signum)
{
	printf("ERROR: SIGPIPE Signal received\n");
}

/*
 * ResourceCleanUp - Function which is called in the end to cleanup all resources
 * This should be a handler for a SIGKILL signal as our application runs infinitely
 * And will be terminated by pressing CTRL-C
 * */


/* Command line arguments
 * nodes.out NodeId PrimeNode TCPPort UDPPort nodeInformationFile connectionInfoFile */

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
	int rv;
	int ix,jx;

	/* Check command line arguments */
	if(argc != 8)
	{
		printf("ERROR: Invalid command line arguments\nnodes.out <<NodeId>> <<PrimeNode>> <<MValue>> <<TCPPort>> <<UDPPort>> <<nodeInfoFile>> <<connectionInfoFile>>\n");
		return -1;
	}

	/* Extract command line arguments */
	/* Store own node information */
	ownNodeId = atoi(argv[1]);
	primeNode = atoi(argv[2]);
	numOfNodesToJoin = atoi(argv[3]);
	strcpy(nodeInformationFile, argv[6]);
	strcpy(connectionInfoFile, argv[7]);

	/* Read the connections file and fill the database */
	if(initializeNodeDB(nodeInformationFile) == -1)
	{
		return -1;
	}

#ifdef DEBUG
	printNodeDB();
#endif // DEBUG

	/* Get lock for NodeDB */
	pthread_mutex_lock(&mutex_nodeDB);

	gethostname(nodeInformation[ownNodeId].hostName, MAX_CHARACTERS_IN_HOSTNAME);
	strcpy(nodeInformation[ownNodeId].tcpPortNumber, argv[4]);
	strcpy(nodeInformation[ownNodeId].udpPortNumber, argv[5]);

	/* Release lock for NodeDB */
	pthread_mutex_unlock(&mutex_nodeDB);

	/* Fill your algorithm data structures */
	if(fillAlgorithmDB(connectionInfoFile, ownNodeId) == -1)
	{
		exit(1);
	}

#if DEBUG
	/* DEBUG */
	printf("Adjacency List:\n");
	for(ix=0 ; ix<AdjList.size() ; ix++)
	{
		for(jx=0 ; jx<AdjList.at(ix).size() ; jx++)
		{
			printf("%d ", AdjList.at(ix).at(jx));
		}
		printf("\n");
	}
#endif

#ifdef DEBUG
	printf("INFO: Id:%d Own host information %s\n", ownNodeId, nodeInformation[ownNodeId].hostName);
#endif // DEBUG

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

	if((rv = sem_init(&sem_connectRespWait, 0, 0)) == -1)
	{
		printf("ERROR: Main, semaphore creation failed for Semaphore - ConnectResp, Error:%s\n", gai_strerror(rv));
		exit(1);
	}

	if((rv = sem_init(&sem_startTcpConListen, 0, 0)) == -1)
	{
		printf("ERROR: Main, semaphore creation failed for Semaphore - TCP Connection, Error:%s\n", gai_strerror(rv));
		exit(1);
	}

#ifdef DEBUG
	/* Debug */
	printDistanceVector();
	printDegreeVector();
	printDVM();
#endif // DEBUG

	/* Register SIGPIPE signal handler */
	signal(SIGPIPE, sigPipeHandler);

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





