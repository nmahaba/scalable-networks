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
extern pthread_mutex_t mutex_nodeDB;
extern SNodeInformation nodeInformation[MAX_NUMBER_OF_NODES];
extern int primeNode;

/*********************************************************************************************************
 /** spawnUdpThreadForQueries: Function to spawn a thread to handle UDP messages for node queries
  *
  *	@param[in] ownNodeId: NodeId of the current node
  *
  *  Block on a UDP Socket for Query message and reply when a query arrives
  *
  * @return -1 if error
 *
 *********************************************************************************************************/
int spawnUdpThreadForQueries(int *ownNodeId)
{
	int status;

	pthread_attr_init(&udpQueryThreadAttr);

	pthread_attr_setdetachstate(&udpQueryThreadAttr, PTHREAD_CREATE_JOINABLE);

	if((status = pthread_create(&udpQueryThread, &udpQueryThreadAttr, handleNodeQueries, (void *)ownNodeId)) == -1)
	{
		printf("ERROR: spawnUdpThreadForQueries, failed spawning a thread, ECODE %s\n", strerror(errno));
		return -1;
	}

	return 0;
}

/*
 * handleFileNodeQueries: Function executes on a new thread and handles the UDP, node query messages
 */
void *handleNodeQueries(void *data)
{
	printf("INFO: UDP Thread for handling Node Query messages\n");

	while(1);
}

/*********************************************************************************************************
 /** spawnUdpThreadForEntryHandler: Function to spawn a thread to handle UDP messages for JoinReq, JoinResp
  *
  *	 @param[in] ownNodeId: NodeId of the current node
  *
  *  Block on a UDP Socket for JoinReq message, do the processing and send the JoinResp message
  *
  * @return -1 if error
 *
 *********************************************************************************************************/
int spawnUdpThreadForEntryHandler(int *ownNodeId)
{
	int status;

	pthread_attr_init(&udpEntryHandlerThreadAttr);

	pthread_attr_setdetachstate(&udpEntryHandlerThreadAttr, PTHREAD_CREATE_JOINABLE);

	if((status = pthread_create(&udpEntryHandlerThread, &udpEntryHandlerThreadAttr, handleNodeEntry, (void*) ownNodeId)) == -1)
	{
		printf("ERROR: spawnUdpThreadForEntryHandler, failed spawning a thread, ECODE %s\n", strerror(errno));
		return -1;
	}

	return 0;
}

int dummyConnectionCreator(int nodeId)
{
	int returngetaddrinfo;
	struct addrinfo tempSocketInfo, *finalSocketInfo, *iSocketLoopIter;

	/* Check if connection has to be established with a particular node */
	/* If connectionInfo[nodeId] is set then a connection has to be established with it*/
	(void) memset(&tempSocketInfo, 0, sizeof tempSocketInfo); 	/* Remember this, this COSTED you much */
	tempSocketInfo.ai_family 	=   AF_UNSPEC; 					/* Don't care IPv4 or IPv6 */
	tempSocketInfo.ai_socktype 	=   SOCK_STREAM; 				/* TCP stream sockets */

	if((returngetaddrinfo = getaddrinfo(nodeInformation[nodeId].hostName, nodeInformation[nodeId].tcpPortNumber, &tempSocketInfo, &finalSocketInfo)) != 0)
	{
		printf("Error: connectToPrimeNodes ECODE %s for %s Port %s\n", strerror(returngetaddrinfo), nodeInformation[nodeId].hostName,
				nodeInformation[nodeId].tcpPortNumber);

		return -1;
	}

	/* Loop through all valid values return by getaddrinfo() */
	for(iSocketLoopIter = finalSocketInfo ; iSocketLoopIter != NULL ; iSocketLoopIter = iSocketLoopIter->ai_next)
	{
		if ((nodeInformation[nodeId].tcpSocketFd = socket(iSocketLoopIter->ai_family, iSocketLoopIter->ai_socktype, iSocketLoopIter->ai_protocol)) == -1)
		{
			perror("Error: Socket connection failed\n");
			continue;
		}

		if (-1 == connect(nodeInformation[nodeId].tcpSocketFd, iSocketLoopIter->ai_addr, iSocketLoopIter->ai_addrlen))
		{
#ifdef DEBUG
			printf("DEBUG: Cannot connect, socketFd %d\n", nodeInformation[nodeId].tcpSocketFd);
#endif // DEBUG

			close(nodeInformation[nodeId].tcpSocketFd);

#ifdef DEBUG
			printf("Node %s CANT connect to %s, ErrorCode %s\n", nodeInformation[ownNodeId].hostName, nodeInformation[ix].hostName, strerror(errno));
#endif
			continue;
		}
		else
		{
			/* Connection established */
			printf("Connected to %s\n", nodeInformation[nodeId].hostName);
		}

		break;
	}

	freeaddrinfo(finalSocketInfo);

	return 0;
}

/*
 * handleNodeEntry: Function executes on a new thread and handles the UDP, node query messages
 */
void *handleNodeEntry(void *data)
{
	int *inData = (int *) data;
	int nodeId = *inData;
	int messageSize = -1;
	mConnectRequest connectRequest;
	mRouteInformation routeInformation;
	char buffer[100]; /* Need to fix the size of this buffer */

	printf("INFO: UDP Thread for handling Node Entry\n");

	/*
	 * If non- prime node, then perform the below
	 * 	1. Send JoinReq to one of the prime nodes
	 * 	2. Wait for JoinResp, then establish TCP connection to m nodes in JoinResp
	 * 	3. Send ConnectReq to m nodes (Using TCP Socket - Need to acquire lock to access nodeDB)
	 * 	4. Wait on the indication from the TCP thread about receiving all m ConnectionResp messages
	 * 	5. After receiving all m ConnectionResp messages, send RouteInformation to m nodes (they are your only neighbors)
	 * 		We need to acquire the lock here also since we are sending RouteInformation using TCP thread accessing nodeDB
	 * 	If prime node, start waiting for JoinReq message from non-prime node
	 * 	1. Use Barbasi model to decide which to which m nodes the non-prime node will connect.
	 * 	2. Send JoinResp with necessary information back to non-prime node.
	 */

	if(primeNode == 0)
	{
		/* Send JoinReq to one of the prime nodes */

		/* Wait for JoinResp from the prime node */

		/* Send ConnectReq to m nodes */
		/* Form the ConnectReq message */
		connectRequest.messageId = ConnectionRequest;

		/* Get lock before accessing the node database */
		pthread_mutex_lock(&mutex_nodeDB);
		connectRequest.nodeInformation.nodeId = nodeId;
		connectRequest.nodeInformation.tcpSocketFd = -1;  /* THis field is not used by the receiver */
		strcpy(connectRequest.nodeInformation.hostName, nodeInformation[nodeId].hostName);
		strcpy(connectRequest.nodeInformation.tcpPortNumber, nodeInformation[nodeId].tcpPortNumber);
		strcpy(connectRequest.nodeInformation.udpPortNumber, nodeInformation[nodeId].udpPortNumber);

		/* The message is formed now, we need to send it to one of the prime node */
		/* For testing purpose send it to the first node */
		messageSize = sizeof(connectRequest);

		memset(buffer, 0, messageSize);					/* Clear the buffer before use */
		memcpy(buffer, &connectRequest, messageSize);	/* Fill buffer */

		/* Debug */
		/* Create a TCP connection with node0 */
		dummyConnectionCreator(1);

		if(-1 == sendDataOnTCP(nodeInformation[1].tcpSocketFd, buffer, &messageSize))
		{
			printf("ERROR: handleNodeEntry, could send only %d bytes\n", messageSize);
			exit(1);
		}

		pthread_mutex_unlock(&mutex_nodeDB);

		/* Testing part 2 */
		/* Send RouteInformation */
		/* Prepare route information */
		routeInformation.messageId = RouteInformation;
		routeInformation.nodeId = nodeId;

		messageSize = sizeof(routeInformation);

		memset(buffer, 0, messageSize);					/* Clear the buffer before use */
		memcpy(buffer, &routeInformation, messageSize);	/* Fill buffer */

		/* Get lock before accessing the node database */
		pthread_mutex_lock(&mutex_nodeDB);

		if(-1 == sendDataOnTCP(nodeInformation[1].tcpSocketFd, buffer, &messageSize))
		{
			printf("ERROR: handleNodeEntry, could send only %d bytes\n", messageSize);
			exit(1);
		}

		pthread_mutex_unlock(&mutex_nodeDB);



		/* Wait until we receive all m ConnectResp messages */



	}

	/* General message handling section */


	while(1);
}

