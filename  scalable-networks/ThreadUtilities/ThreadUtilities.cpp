/*
 * ThreadUtilities.c
 *
 *  Created on: Nov 15, 2013
 *      Author: Niranjan
 */
#include <ThreadUtilities.h>
#include <pthread.h>

using namespace std;

/* Extern */
extern pthread_attr_t udpQueryThreadAttr;
extern pthread_t udpQueryThread;
extern pthread_attr_t udpEntryHandlerThreadAttr;
extern pthread_t udpEntryHandlerThread;
extern pthread_mutex_t mutex_nodeDB;
extern SNodeInformation nodeInformation[MAX_NUMBER_OF_NODES];
extern int primeNode;
extern sem_t sem_connectRespWait;
extern sem_t sem_startTcpConListen;
extern int numOfPrimeNodes;
extern vector<int> barbasiBag;								/* Used for Barbasi model */

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

/*
 * handleNodeEntry: Function executes on a new thread and handles the UDP, node query messages
 */
void *handleNodeEntry(void *data)
{
	int *inData = (int *) data;
	int ownNodeId = *inData;
	int messageSize = -1;
	mConnectRequest connectRequest;
	mRouteInformation routeInformation;
	mJoinResponse joinResponse;
	mJoinRequest joinRequest;
	char buffer[100]; /* Need to fix the size of this buffer */
	struct sockaddr senderAddress;
	socklen_t addr_len;
	int recvBytes;
	int sentBytes;
	int socketfd;
	int ix;
	int nodeId;
	int numOfConnectionRespReceived = 0;
	int rv;
	int node1, node2;
	vector<int> mSet;
	int randomIndex;
	int candidateNodeId;

	printf("INFO: UDP Thread for handling Node Entry\n");
	srand(time(NULL));

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
		/* Send JoinReq randomly to some prime node - Below its always sent to the first node */
		/* << Place Holder >> */

		/* Send JoinReq to one of the prime nodes and wait for JoinResp from it */
		if(sendJoinReq(1, ownNodeId, &joinResponse) == -1)
		{
			exit(0);
		}

		/* Send m ConnectReq messages */
		/* For loop for sending m ConnectReq messages */
		for(ix=0 ; ix<joinResponse.nodeCount ; ix++)
		{
			if(sendConnectReq(joinResponse.nodeInformation[ix].nodeId, ownNodeId) == -1)
			{
				exit(1);
			}
		}

		/* Wait until we receive all m ConnectResp messages */
		/* while loop for waiting for m ConnectResp messages */

		numOfConnectionRespReceived = 0;

		while(numOfConnectionRespReceived != joinResponse.nodeCount)
		{
			if((rv = sem_wait(&sem_connectRespWait)) == -1)
			{
				printf("ERROR: handleNodeEntry, semaphore failed while waiting, Error:%s\n", gai_strerror(rv));
				exit(1);
			}

			numOfConnectionRespReceived++;
		}

		printf("DEBUG: Received all %d ConnectResponse\n", joinResponse.nodeCount);

		/* Send the first RouteInformation message to neighbor nodes */
		/* For loop for sending RouteInformation for M neighbors */
		for(ix=0 ; ix<joinResponse.nodeCount ; ix++)
		{
			if(sendRouteUpdate(joinResponse.nodeInformation[ix].nodeId, ownNodeId, -1) == -1)
			{
				exit(1);
			}
		}
	}
	else
	{
		/* Create a socket for listening to messages on UDP */
		if((socketfd = createSocketAndBindAddress(ownNodeId, UDP_CONNECTION, SERVER, 0)) == -1)
		{
			printf("ERROR: handleNodeEntry, failed to create socket for UDP connections, error: %s", gai_strerror(socketfd));
			exit(1);
		}
		else
		{
			printf("DEBUG: handleNodeEntry, fd:%d created for UDP message handling\n", socketfd);
		}

		/* General message handling section */
		while(true)
		{
			addr_len = sizeof(senderAddress);

#ifdef DEBUG
			printf("Waiting for messages...\n");
#endif //DEBUG

			if ((recvBytes = recvfrom(socketfd, &joinRequest, sizeof(mJoinRequest), 0, &senderAddress, &addr_len)) == -1)
			{
				printf("ERROR: handleNodeEntry, recvfrom failed, Error:%s\n", gai_strerror(recvBytes));
				exit(1);
			}

			printf("DEBUG: Received JoinRequest from %s, NodeId:%d\n",
					joinRequest.nodeInformation.hostName,
					joinRequest.nodeInformation.nodeId);

			/********************************************************************************************************
			 ********************************************************************************************************
							Barbasi Model Intelligence should be added here - START
			 ********************************************************************************************************
			 ********************************************************************************************************/

			/* ToDo List
			 * 1. Implement Barbasi model
			 * 2. Make M as a configurable parameter, the user should be able to specify this in runtime
			 *    Create a global variable, read it during startup and use it in the place of the MACRO NODES_TO_JOIN
			 * */

			/* For the time being select 2 nodes, node 4 and node 5 for new connection */
			/* Prepare the message to be sent */
			joinResponse.messageId = JoinResponse;
			joinResponse.nodeCount = NODES_TO_JOIN;				/* M is 2, check Constants.h file */

			/* Clear M set */
			mSet.clear();

			while(true)
			{
				if(mSet.size() == NODES_TO_JOIN)
				{
					break;
				}

				randomIndex = getRandomNumber(0, barbasiBag.size());

				/* Safety check */
				if(randomIndex >= barbasiBag.size())
				{
					continue;
				}

				if(find(mSet.begin(), mSet.end(), barbasiBag.at(randomIndex)) != mSet.end())
				{
					continue;
				}

				mSet.push_back(barbasiBag.at(randomIndex));
			}

			for(int ix=0 ; ix<NODES_TO_JOIN ; ix++)
			{
				candidateNodeId = mSet.at(ix);

				printf("DEBUG: Winner NodeId:%d Host:%s\n", candidateNodeId, nodeInformation[candidateNodeId].hostName);

				joinResponse.nodeInformation[ix].nodeId = nodeInformation[candidateNodeId].nodeId;
				strcpy(joinResponse.nodeInformation[ix].hostName,nodeInformation[candidateNodeId].hostName);
				strcpy(joinResponse.nodeInformation[ix].tcpPortNumber, nodeInformation[candidateNodeId].tcpPortNumber);
				strcpy(joinResponse.nodeInformation[ix].udpPortNumber, nodeInformation[candidateNodeId].udpPortNumber);
			}

			/********************************************************************************************************
			 ********************************************************************************************************
							Barbasi Model Intelligence should be added here - END
			 ********************************************************************************************************
			 ********************************************************************************************************/

			/* Send the message */
			if((sentBytes = sendto(socketfd, &joinResponse, sizeof(joinResponse), 0, &senderAddress, addr_len)) == -1)
			{
				printf("ERROR: handleNodeEntry, Can't send JoinResponse message, ECODE: %s\n", gai_strerror(sentBytes));
				exit(1);
			}
			else
			{
				printf("INFO: JoinResponse sent to NodeId:%d Host:%s\n",
						joinRequest.nodeInformation.nodeId,
						joinRequest.nodeInformation.hostName);
			}
		}
	}

	printf("INFO: UDP Thread exiting\n");
}

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
int getRandomNumber(int min, int max)
{
	return((rand() % (max+1-min))+ min);
}
