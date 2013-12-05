#include <SocketUtilities.h>

//#define DEBUG

using namespace std;

/* EXTERN declarations - START 	*/
extern SNodeInformation nodeInformation[MAX_NUMBER_OF_NODES];
extern int connectionInfo[MAX_NUMBER_OF_NODES];
extern pthread_mutex_t mutex_nodeDB;
extern sem_t sem_connectRespWait;
extern sem_t sem_startTcpConListen;
extern int primeNode;
extern int DistV[MAX_NUMBER_OF_NODES];					 	/* The distance vector */
extern int DegV[MAX_NUMBER_OF_NODES];						/* The degree vector */
extern int DVM[MAX_NUMBER_OF_NODES][MAX_NUMBER_OF_NODES]; 	/* The distance vector matrix */
extern vector<int> barbasiBag;								/* Used for Barbasi model */
/* EXTERN declarations - END 	*/

static void printNodeDB()
{
	int index = 0;

	/* Get lock for NodeDB */
	//pthread_mutex_lock(&mutex_nodeDB);

	printf("*****************************************\n");
	printf("NodeId\tHostName\tTCPPort\tUDPPort\tTCPSocket\n");
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
	//pthread_mutex_unlock(&mutex_nodeDB);
}

/* Debug */
void printDistanceVector()
{
	int ix;

	printf("Updated Distance Vector\n");

	for(ix=0 ; ix<MAX_NUMBER_OF_NODES ; ix++)
	{
		printf("%d ", DistV[ix]);
	}
	printf("\n");
}

/* Debug */
void printDegreeVector()
{
	int ix;

	printf("Updated Degree Vector\n");

	for(ix=0 ; ix<MAX_NUMBER_OF_NODES ; ix++)
	{
		printf("%d ", DegV[ix]);
	}
	printf("\n");
}

/* Debug */
void printDVM()
{
	int ix,jx;

	printf("Updated DVM\n");

	for(ix=0 ; ix<MAX_NUMBER_OF_NODES ; ix++)
	{
		for(jx=0 ; jx<MAX_NUMBER_OF_NODES ; jx++)
		{
			printf("%d\t", DVM[ix][jx]);
		}
		printf("\n");
	}
	printf("\n");
}

/****************************************************************************************
 /** sendDataOnTCP: Function to send data completely
  *
  * @param[in] socketFd This is the file descriptor to which data has to be written
  *
  * @param[in] *sendBuffer Pointer to the message
  *
  * @param[in/out] length This is the length of the data to be sent, which is also returned back
  *
  * @return 0 if PASSED or -1 if FAILED
 *
 ****************************************************************************************/
int sendDataOnTCP(int socketFd, char *sendBuffer, int *length)
{
	int total = 0; // how many bytes we've sent
	int bytesleft = *length; // how many we have left to send
	int n;

	while(total < *length)
	{
		n = send(socketFd, sendBuffer+total, bytesleft, 0);

		if (n == -1)
		{
			printf("ERROR: sendAll, send failed, fd:%d ECODE %s\n", socketFd, strerror(errno));
			break;
		}

		total += n;
		bytesleft -= n;
	}

	*length = total; /* Return number actually sent here */

	return (n==-1)?-1:0; /* return -1 on failure, 0 on success */
}

/****************************************************************************************
 /** receiveDataOnTCP: Function to send data completely
  *
  * @param[in] socketFd This is the file descriptor to which data has to be written
  *
  * @param[in] *receiveBuffer Pointer to the receiver buffer
  *
  * @param[in/out] length This is the length of the data to be sent, which is also returned back
  *
  * @return 0 if PASSED or -1 if FAILED
 *
 ****************************************************************************************/
int receiveDataOnTCP(int socketFd, char *receiveBuffer, int *length)
{
	int total = 0; 				// How many bytes we've received
	int bytesleft = *length; 	// How many we have left to receive
	int n;

	while(total < *length)
	{
		n = recv(socketFd, receiveBuffer+total, bytesleft, 0);

		if (n == -1)
		{
			printf("ERROR: receiveAll, recv failed, ECODE %s\n", strerror(errno));
			break;
		}

		total += n;
		bytesleft -= n;
	}

	*length = total; /* Return number actually received here */

	return (n==-1)?-1:0; /* return -1 on failure, 0 on success */
}

/****************************************************************************************
 /** connectToPrimeNodes: Function to establish connection with prime nodes using the
  * 						connection information
   * @param[in] ownNodeId Id of current node
 ****************************************************************************************/
int connectToPrimeNodes(int ownNodeId)
{
	int ix;
	int returngetaddrinfo;
	struct addrinfo tempSocketInfo, *finalSocketInfo, *iSocketLoopIter;

	/* Get lock for NodeDB */
	pthread_mutex_lock(&mutex_nodeDB);

	for(ix=1 ; ix<MAX_NUMBER_OF_NODES ; ix++)
	{
		/* Check if connection has to be established with a particular node */
		/* If connectionInfo[nodeId] is set then a connection has to be established with it*/
		if(connectionInfo[ix] == 1)
		{
			(void) memset(&tempSocketInfo, 0, sizeof tempSocketInfo); 	/* Remember this, this COSTED you much */
			tempSocketInfo.ai_family 	=   AF_UNSPEC; 					/* Don't care IPv4 or IPv6 */
			tempSocketInfo.ai_socktype 	=   SOCK_STREAM; 				/* TCP stream sockets */

			if((returngetaddrinfo = getaddrinfo(nodeInformation[ix].hostName, nodeInformation[ix].tcpPortNumber, &tempSocketInfo, &finalSocketInfo)) != 0)
			{
				printf("Error: connectToPrimeNodes ECODE %s for %s Port %s\n", strerror(errno), nodeInformation[ix].hostName,
						nodeInformation[ix].tcpPortNumber);

				return -1;
			}

			/* Loop through all valid values return by getaddrinfo() */
			for(iSocketLoopIter = finalSocketInfo ; iSocketLoopIter != NULL ; iSocketLoopIter = iSocketLoopIter->ai_next)
			{
				if ((nodeInformation[ix].tcpSocketFd = socket(iSocketLoopIter->ai_family, iSocketLoopIter->ai_socktype, iSocketLoopIter->ai_protocol)) == -1)
				{
					perror("Error: Socket connection failed\n");
					continue;
				}

				if (-1 == connect(nodeInformation[ix].tcpSocketFd, iSocketLoopIter->ai_addr, iSocketLoopIter->ai_addrlen))
				{
#ifdef DEBUG
					printf("DEBUG: Cannot connect, socketFd %d\n", nodeInformation[ix].tcpSocketFd);
#endif // DEBUG

					close(nodeInformation[ix].tcpSocketFd);

					/* Reset the value */
					nodeInformation[ix].tcpSocketFd = -1;

#ifdef DEBUG
					printf("Node %s CANT connect to %s, ErrorCode %s\n", nodeInformation[ownNodeId].hostName, nodeInformation[ix].hostName, strerror(errno));
#endif
					continue;
				}
				else
				{
					/* Connection established */
					printf("Connected to %s\n", nodeInformation[ix].hostName);
				}

				break;
			}

			freeaddrinfo(finalSocketInfo);
		}
	}

	/* Release lock for NodeDB */
	pthread_mutex_unlock(&mutex_nodeDB);

	return 0;
}

/****************************************************************************************
 /** processTCPConnections: Entry function which waits for new TCP connections or process
  * 						queries on TCP connections
  * @param[in] nodeId Id of the current node
 *
 ****************************************************************************************/
int processTCPConnections(int ownNodeId)
{
	int ix;
	int listener; 					/* Node's listening file descriptor */
	int fdmax; 						/* Maximum file descriptor number */
	int newfd; 						/* Newly accept()ed socket descriptor */
	int numberOfBytesReceived = 0;  /* Number of bytes received by recv - If 0 then node has terminated connection */
	fd_set master; 					/* Master file descriptor list */
	fd_set read_fds; 				/* Temp file descriptor list for select() */
	char buffer[200];
	int messageSize	= -1;
	eMessageId messageId;
	int rv;
	int nodeId;
	int jx;
	int routeUpdateResult;

	socklen_t addrlen;
	struct sockaddr_storage remoteaddr; /* Client address */

	/* Create a socket, bind a address to the socket and get a file descriptor to listen to connections  */
	listener = createSocketAndBindAddress(ownNodeId, TCP_CONNECTION, SERVER, 0);

#ifdef DEBUG
	printf("DEBUG: Listening socket %d\n", listener);
#endif // DEBUG

	if(listener == -1)
	{
		exit(1);
	}

	/* Listen to other nodes */
	if (listen(listener, MAX_NUMBER_OF_NODES) == -1)
	{
		perror("Error: processTCPConnections, listen failed\n");
		return -1;
	}

	FD_ZERO(&master); 	/* Clear the master and temp sets */
	FD_ZERO(&read_fds);
#ifdef DEBUG
	printf("DEBUG: FD Sets cleared\n");
#endif // DEBUG

	/* Add the listener to the master set */
	FD_SET(listener, &master);

#ifdef DEBUG
	printf("DEBUG: fd:%d set\n", listener);
#endif // DEBUG

	/* Keep track of the biggest file descriptor */
	fdmax = listener;

	if(primeNode == 0)
	{
		/* Wait here until the UDP thread sets up TCP connections and updates the FD's if its a non-prime node */
		if((rv = sem_wait(&sem_startTcpConListen)) == -1)
		{
			printf("ERROR: processTCPConnections, semaphore failed while waiting, Error:%s\n", gai_strerror(rv));
			exit(1);
		}
	}

	/* Get lock for NodeDB */
	pthread_mutex_lock(&mutex_nodeDB);

	/* Add the file descriptors obtained during connect() to the master list so that nodes can also listen to data sent by other nodes */
	for(ix=1 ; ix<MAX_NUMBER_OF_NODES ; ix++)
	{
		if(nodeInformation[ix].tcpSocketFd != -1)
		{
#ifdef DEBUG
			printf("DEBUG: fd:%d set Id:%d\n", nodeInformation[ix].tcpSocketFd, ix);
#endif // DEBUG

			FD_SET(nodeInformation[ix].tcpSocketFd, &master);
		}

		/* Update fdmax with the highest file descriptor in nodeInformation */
		if(nodeInformation[ix].tcpSocketFd > fdmax)
		{
			fdmax = nodeInformation[ix].tcpSocketFd;
		}
	}

#ifdef DEBUG
	/* Check this ------- BUG */
	/* Store current node's listening file descriptor in nodeInformation in own NodeId's location */
	nodeInformation[ownNodeId].tcpSocketFd 	= listener;
#endif // DEBUG

	/* Release lock for NodeDB */
	pthread_mutex_unlock(&mutex_nodeDB);

	/* Main loop */
	while(true)
	{
#ifdef DEBUG
		printf("DEBUG: Listening.....\n");
#endif // DEBUG

		read_fds = master; /* Copy it */

		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
		{
			printf("Error processTCPConnections, select failed ECode %s\n", strerror(errno));
			return -1;
		}

		/* Run through the existing connections looking for data to read */
		/* Tried to fix a bug, run through the for loop untill START message is received - WORKED */
		for(ix=0 ; ix<=fdmax ; ix++)
		{
			if (FD_ISSET(ix, &read_fds))
			{
				/* We got one !!! */
				if (ix == listener)
				{
					/* Handle new connections */
					addrlen = sizeof remoteaddr;

					newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);

					if (newfd == -1)
					{
						printf("ERROR: processTCPConnections, accept failed\n");
					}
					else
					{
						/*
						 * Update the new file descriptor in node's database which will used for future communication
						 */

#ifdef DEBUG
						printf("DEBUG: New connection established, fd:%d\n", newfd);
#endif // DEBUG

						if(updateScoketDescInNodeDB((struct sockaddr *)&remoteaddr, newfd) == -1)
						{
							exit(1);
						}

#ifdef DEBUG
						printf("DEBUG: fd:%d set\n", newfd);
#endif // DEBUG

						FD_SET(newfd, &master); /* Add to master set */

						if (newfd > fdmax)
						{
							/* Keep track of the max */
							fdmax = newfd;
						}
					}
				}
				else
				{
#ifdef DEBUG
					printf("DEBUG: Data received by the node\n");
#endif // DEBUG

					memset(buffer, 0, sizeof(buffer));			/* Clear buffer before use */

					/* Read just the message type - 4 bytes */
					recv(ix, &messageId, sizeof(messageId), 0);

					memcpy(buffer, &messageId, sizeof(messageId));

#ifdef DEBUG
					printf("DEBUG: Received message %d, Socket:%d\n", messageId, ix);
#endif // DEBUG

					if(messageId == ConnectionRequest)
					{
						mConnectResponse connectResponse;
						mConnectRequest connectRequest;

						messageSize = sizeof(connectRequest) - sizeof(messageId);

						if(receiveDataOnTCP(ix, buffer+sizeof(messageId), &messageSize) == -1)
						{
							printf("ERROR: processTCPConnections, ConnectionRequest could receive only %d bytes\n", messageSize);
							exit(1);
						}

						memcpy(&connectRequest, buffer, messageSize);

						/* Extract nodeId */
						nodeId =	connectRequest.nodeInformation.nodeId;

						printf("INFO: ConnectRequest received from NodeId:%d Host:%s\n",
								nodeId,
								connectRequest.nodeInformation.hostName);

						/* Update your database */
						/* Get lock before accessing node database */
						pthread_mutex_lock(&mutex_nodeDB);

#ifdef DEBUG
						printf("DEBUG: UpdateNodeDb, NodeId:%d Socket:%d\n", nodeId, nodeInformation[nodeId].tcpSocketFd);
#endif // DEBUG

						/* Store socket information in Node Database */
						nodeInformation[nodeId].nodeId 		= nodeId;													/* NodeId */
						nodeInformation[nodeId].tcpSocketFd = ix;														/* SocketFd */
						strcpy(nodeInformation[nodeId].hostName, connectRequest.nodeInformation.hostName);				/* HostName */
						strcpy(nodeInformation[nodeId].tcpPortNumber, connectRequest.nodeInformation.tcpPortNumber);	/* TCP Port */
						strcpy(nodeInformation[nodeId].udpPortNumber, connectRequest.nodeInformation.udpPortNumber);	/* UDP Port */

						/* Release lock after accessing node database */
						pthread_mutex_unlock(&mutex_nodeDB);

						/* Send connection response */
						/* Clear buffer */
						memset(buffer,0,sizeof(buffer));

						/* Form the message */
						connectResponse.messageId 					= ConnectionResponse;
						connectResponse.nodeId						= ownNodeId;
						connectResponse.routeInformation.nodeId 	= ownNodeId;
						connectResponse.routeInformation.messageId 	= RouteInformation;

						/* Fill RouteInformation */
						memcpy(connectResponse.routeInformation.newDegV, DegV, sizeof(DegV));
						memcpy(connectResponse.routeInformation.newDistV, DistV, sizeof(DistV));

						/* Get the size */
						messageSize = sizeof(connectResponse);

						memcpy(buffer, &connectResponse, messageSize);

						if((sendDataOnTCP(ix, buffer, &messageSize)) == -1)
						{
							printf("ERROR: processTCPConnections, ConnectionResponse could send only (%d/%d) bytes\n",
									messageSize,
									sizeof(connectResponse));

							exit(1);
						}
						else
						{
							printf("DEBUG: ConnectionResp sent to NodeId:%d, (%d/%d) bytes\n",
									nodeId,
									messageSize,
									sizeof(connectResponse));
						}
					}
					else if(messageId == RouteInformation)
					{
						mRouteInformation routeInformation;

						messageSize = sizeof(routeInformation) - sizeof(messageId);

						if(receiveDataOnTCP(ix, buffer+sizeof(messageId), &messageSize) == -1)
						{
							printf("ERROR: processTCPConnections, could receive only %d bytes\n", messageSize);
							exit(1);
						}

						memcpy(&routeInformation, buffer, messageSize);

						printf("INFO: RouteInformation received NodeId:%d\n", routeInformation.nodeId);

						/*
						 *	Update your routing table with information from neighbor nodes
						 *	If any route has changed OR degree of any node has changed
						 *		Send RouteInformation to neighbors
						 *	If not, ignore
						 */

						routeUpdateResult = updateDVM(routeInformation, routineUpdate, ownNodeId);
#ifdef DEBUG
//#if 1
						printf("DEBUG: Barbasi Bag contents:\n");
						for(int ax=0 ; ax<barbasiBag.size() ; ax++)
						{
							printf("%d ", barbasiBag.at(ax));
						}
						printf("\n");
#endif // DEBUG

						/* Display the result of the routing update */
						switch(routeUpdateResult)
						{
							case 0:	printf("INFO: No network change\n");
									break;
							case 1: printf("INFO: Degree vector changed\n");
									break;
							case 2: printf("INFO: Distance vector changed\n");
									break;
							case 3: printf("INFO: Degree and Distance vector changed\n");
									break;
						}

						if(routeUpdateResult > 0)
						{
							/* Update the database */
							int originatorNodeId = routeInformation.nodeInformation.nodeId;

							nodeInformation[originatorNodeId].nodeId = originatorNodeId;
							strcpy(nodeInformation[originatorNodeId].hostName, routeInformation.nodeInformation.hostName);			/* HostName */
							strcpy(nodeInformation[originatorNodeId].tcpPortNumber, routeInformation.nodeInformation.tcpPortNumber);	/* TCP Port */
							strcpy(nodeInformation[originatorNodeId].udpPortNumber, routeInformation.nodeInformation.udpPortNumber);	/* UDP Port */
//#ifdef DEBUG
#if 1
							/* DEBUG - Print Distance Vector and Degree Vector */
							printDistanceVector();

							printDegreeVector();
#endif // DEBUG

							/* Get lock for NodeDB */
							pthread_mutex_lock(&mutex_nodeDB);

							/* For the time being I am sending RouteInformation to all my neighbors always */
							for(jx=1 ; jx<MAX_NUMBER_OF_NODES ; jx++)
							{
								/* Don't send to yourself and the source of the message */
								if((nodeInformation[jx].nodeId != ownNodeId) && (nodeInformation[jx].nodeId != routeInformation.nodeId) && (nodeInformation[jx].tcpSocketFd != -1))
								{
									if(sendRouteUpdate(nodeInformation[jx].nodeId, ownNodeId, routeInformation.nodeId) == -1)
									{
										exit(1);
									}
								}
							}

							/* Release lock for NodeDB */
							pthread_mutex_unlock(&mutex_nodeDB);
						}
					}
					else if(messageId == ConnectionResponse)
					{
						mConnectResponse connectResponse;

						messageSize = sizeof(connectResponse) - sizeof(messageId);

						if(receiveDataOnTCP(ix, buffer+sizeof(messageId), &messageSize) == -1)
						{
							printf("ERROR: processTCPConnections, could receive only %d bytes\n", messageSize);
							exit(1);
						}

						memcpy(&connectResponse, buffer, messageSize);

						printf("DEBUG: Received connection response from NodeId:%d\n",connectResponse.nodeId);

						/*
						 * Update your database here.....
						 * 1. Distance Vector.
						 * 2. Degree Vector
						 */

						routeUpdateResult = updateDVM(connectResponse.routeInformation, newNodeJoinUpdate, ownNodeId);

#ifdef DEBUG
//#if 1
						printf("DEBUG: Barbasi Bag contents:\n");
						for(int ax=0 ; ax<barbasiBag.size() ; ax++)
						{
							printf("%d ", barbasiBag.at(ax));
						}
						printf("\n");
#endif // DEBUG

						/* Signal the UDP thread */
						if((rv = sem_post(&sem_connectRespWait)) == -1)
						{
							printf("ERROR: processTCPConnections, semaphore failed while posting, ECODE:%s\n",
									gai_strerror(rv));
							exit(1);
						}
					}
					else
					{
						printf("ERROR: processTCPConnections, Incorrect mesgId:%d, fd:%d\n", messageId, ix);
					}
				}
			} /* END got new incoming connection */
		} /* END looping through file descriptors */
	} /* END while - All nodes have communication channels established */
}

/*********************************************************************************************************
 /** updateScoketDescInNodeDB: Function to update the file descriptor of the newly connected
  *  node in our database. It does the below
  *
  *  	1. Find the domain name of the new node using getnameinfo API
  *  	2. Go through the list of node information and when the matching node is found update its
  *  	   file descriptor
  *
  *	@param[in] pSockAddr Socket descriptor of the newly added node
  *
  * @return -1 if error
 *
 *********************************************************************************************************/
int updateScoketDescInNodeDB(struct sockaddr *pSockAddr, int fileDescriptor)
{
	int ix;
	int returnValue;
	char newNodeHostName[MAX_CHARACTERS_IN_HOSTNAME];

	returnValue = getnameinfo(pSockAddr, sizeof (struct sockaddr), newNodeHostName, MAX_CHARACTERS_IN_HOSTNAME, 0, 0, 0);

	printf("INFO: %s connected\n", newNodeHostName);

	if(returnValue == -1)
	{
		printf("ERROR: updateFileDescriptorInNodeInformation, getnameinfo failed\n");
		return -1;
	}

	/* Get lock for NodeDB */
	pthread_mutex_lock(&mutex_nodeDB);

	for(ix=1 ; ix<MAX_NUMBER_OF_NODES ; ix++)
	{
		if((strcmp(nodeInformation[ix].hostName, newNodeHostName)) == 0)
		{
#ifdef DEBUG
			printf("updateFileDescriptorInNodeInformation: Match found for %s \n", newNodeHostName);
#endif
			nodeInformation[ix].tcpSocketFd = fileDescriptor;

			break;
		}
	}

	/* Release lock for NodeDB */
	pthread_mutex_unlock(&mutex_nodeDB);

	return 0;
}

/*************************************************************************
 /** createSocketAndBindAddress: Function to create a socket to listen and bind address
  *
  * @param[in] ownNodeId: Id of the current node
  *
  * @param[in] connectionType: TCP or UDP connection
  *
  * @param[in] isServer: Indicates if the UDP connection is server or client
  *
  * @param[in] toNodeId: If the connection is for a UDP client, this mentions the destination nodeId
  *
  * @return -1 if error
  *
 ************************************************************************/
int createSocketAndBindAddress(int ownNodeId, int connectionType, int isServer, int toNodeId)
{
	int listener;
	int reUsePort = 1;
	struct addrinfo tempSocketInfo, *finalSocketInfo, *iSocketLoopIter;
	int rv;
	char ownTcpPortNumber[MAX_CHARACTERS_IN_PORTNUMBER];
	char ownUdpPortNumber[MAX_CHARACTERS_IN_PORTNUMBER];
	char toUdpPortNumber[MAX_CHARACTERS_IN_PORTNUMBER];
	char toHostName[MAX_CHARACTERS_IN_HOSTNAME];

	/* Get lock for NodeDB */
	pthread_mutex_lock(&mutex_nodeDB);

	strcpy(ownTcpPortNumber, nodeInformation[ownNodeId].tcpPortNumber);
	strcpy(toUdpPortNumber, nodeInformation[toNodeId].udpPortNumber);
	strcpy(toHostName, nodeInformation[toNodeId].hostName);
	strcpy(ownUdpPortNumber, nodeInformation[ownNodeId].udpPortNumber);

	/* Release lock for NodeDB */
	pthread_mutex_unlock(&mutex_nodeDB);

	(void) memset(&tempSocketInfo, 0, sizeof tempSocketInfo);

	tempSocketInfo.ai_family 	= AF_UNSPEC; 			/* Don't care IPv4 or IPv6 */

	if(connectionType == TCP_CONNECTION)
	{
		/* TCP Connection */
		tempSocketInfo.ai_socktype 	= SOCK_STREAM;	 		/* TCP stream sockets */
		tempSocketInfo.ai_flags 	= AI_PASSIVE;			/* Fill IP Address for you */

		if((rv = getaddrinfo(NULL, ownTcpPortNumber, &tempSocketInfo, &finalSocketInfo)) != 0)
		{
			printf("ERROR: createSocketAndBindAddress, getaddrinfo failed, Error: %s, Connection:%d\n",
					gai_strerror(rv),
					connectionType);

			return -1;
		}
	}
	else
	{
		/* UDP Connection */
		tempSocketInfo.ai_socktype 	= SOCK_DGRAM;	 		/* UDP sockets */

		if(isServer == CLIENT)
		{
			if((rv = getaddrinfo( toHostName, toUdpPortNumber, &tempSocketInfo, &finalSocketInfo)) != 0)
			{
				printf("ERROR: createSocketAndBindAddress, getaddrinfo failed, Error: %s, Connection:%d\n",
						gai_strerror(rv),
						connectionType);

				return -1;
			}
		}
		else
		{
			/* UDP - Server */
			tempSocketInfo.ai_flags 	= AI_PASSIVE;			/* Fill IP Address for you */

#ifdef DEBUG
			printf("DEBUG: Server listening on port:%s\n", nodeInformation[ownNodeId].udpPortNumber);
#endif //DEBUG

			if((rv = getaddrinfo(NULL, ownUdpPortNumber, &tempSocketInfo, &finalSocketInfo)) != 0)
			{
				printf("ERROR: createSocketAndBindAddress, getaddrinfo failed, Error: %s, Connection:%d\n",
						gai_strerror(rv),
						connectionType);

				return -1;
			}
		}
	}

	/* Loop through all valid values return by getaddrinfo() */
	for(iSocketLoopIter = finalSocketInfo ; iSocketLoopIter != NULL ; iSocketLoopIter = iSocketLoopIter->ai_next)
	{
		listener = socket(iSocketLoopIter->ai_family, iSocketLoopIter->ai_socktype, iSocketLoopIter->ai_protocol);

		if (listener < 0)
		{
			continue;
		}

		/* Changing socket options to make sure the ports can be reused */
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &reUsePort, sizeof(int));

		/* We need bind only for TCP connection */
		if(isServer == SERVER)
		{
			if (bind(listener, iSocketLoopIter->ai_addr, iSocketLoopIter->ai_addrlen) < 0)
			{
				close(listener);
				continue;
			}
		}
		break;
	}

	/* if we got here, it means we didn't get bound */
	if (iSocketLoopIter == NULL)
	{
		printf("Error: createSocketAndBindAddress, failed to bind, ECode %s\n", strerror(errno));
		return -1;
	}

	freeaddrinfo(finalSocketInfo);

	return listener;
}

/****************************************************************************************
 /** sendJoinReq: Function to send join request to m nodes
  *
  * @param[in] toNodeId: The node ID of the node which is the recipient of JoinReq message
  *
  * @param[in] ownNodeId: Own node Id
  *
  * @param[in] *SJoinResponse: This structure should be populated by the function
  *                            which has information about the nodes to which it should
  *                            establish TCP connections
  *
  * @return 0 if PASSED or -1 if FAILED
 *
 ****************************************************************************************/
int sendJoinReq(int toNodeId, int ownNodeId, mJoinResponse* joinResponse)
{
	char 	toHostName[MAX_CHARACTERS_IN_HOSTNAME]; 			/* Host name of the to node */
	char	ownHostName[MAX_CHARACTERS_IN_HOSTNAME]; 			/* Host name of the own node */
	char 	toUdpPortNumber[MAX_CHARACTERS_IN_PORTNUMBER];        /* Port number of the node */
	int 	udpSockfd;
	int 	tcpSocket;
	struct 	addrinfo hints, *servinfo, *p;
	int 	rv;
	int 	numbytes;
	int 	ix;
	int		nodeId;
	mJoinRequest joinRequest;

	/* Get lock before accessing node database */
	pthread_mutex_lock(&mutex_nodeDB);

	strcpy(toHostName, nodeInformation[toNodeId].hostName);
	strcpy(ownHostName, nodeInformation[ownNodeId].hostName);
	strcpy(toUdpPortNumber, nodeInformation[toNodeId].udpPortNumber);

	pthread_mutex_unlock(&mutex_nodeDB);

//#ifdef DEBUG
	printf("DEBUG: sendJoinReq, NodeId:%d host:%s, Port:%s\n", toNodeId, nodeInformation[toNodeId].hostName, nodeInformation[toNodeId].udpPortNumber);
#ifdef DEBUG
	printf("DEBUG: sendJoinReq, NodeId:%d host:%s, Port:%s\n", toNodeId, toHostName, toUdpPortNumber);
#endif // DEBUG

	memset(&hints, 0, sizeof hints);
	hints.ai_family 	= AF_UNSPEC;
	hints.ai_socktype 	= SOCK_DGRAM;
	hints.ai_flags		= AI_PASSIVE;				/* Use my IP */

	if ((rv = getaddrinfo(toHostName, toUdpPortNumber, &hints, &servinfo)) != 0)
	{
		printf("ERROR: sendJoinReq, Host:%s UDPPort:%s getaddrinfo: ECODE: %s, Return Value:%d, ERROR:%s\n",
				toHostName,
				toUdpPortNumber,
				strerror(errno),
				rv,
				gai_strerror(rv));

		return -1;
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((udpSockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			printf("DEBUG: Missed a socket\n");
			continue;
		}

		break;
	}

	if (p == NULL)
	{
		printf("ERROR: sendJoinReq: failed to bind socket\n");
		return -1;
	}

	/* Form the Join Message - Only the message and the node information is important */
	joinRequest.messageId = JoinRequest;
	joinRequest.nodeInformation.nodeId = ownNodeId;
	strcpy(joinRequest.nodeInformation.hostName,ownHostName);
	strcpy(joinRequest.nodeInformation.tcpPortNumber,"");
	strcpy(joinRequest.nodeInformation.udpPortNumber,"");
	joinRequest.nodeInformation.tcpSocketFd = -1;

#ifdef DEBUG
	printf("DEBUG: ID:%d Sending JoinReq, contents Name:%s\n", ownNodeId, ownHostName);
#endif // DEBUG

	/* Send the JoinReq message */
	if ((numbytes = sendto(udpSockfd, &joinRequest, sizeof(joinRequest), 0, p->ai_addr, p->ai_addrlen)) == -1)
	{
		printf("ERROR: sendJoinReq, failed to send JoinReq (%d/%d) bytes sent, %s",
				numbytes,
				sizeof(joinRequest),
				gai_strerror(numbytes));

		return -1;
	}
	else
	{
		printf("DEBUG: JoinReq sent to %s, sent (%d/%d) bytes\n", toHostName, numbytes, sizeof(joinRequest));
	}

	/* Now block on receive and return back to the sender */
	if ((numbytes = recvfrom(udpSockfd, joinResponse, sizeof(mJoinResponse) , 0, 0, 0)) == -1)
	{
		printf("ERROR: sendJoinReq, failure in receiving, received (%d/%d) bytes, error %s",
				numbytes,
				sizeof(mJoinResponse),
				strerror(errno));

		return -1;
	}
	else
	{
		printf("DEBUG: JoinResponse message received. M:%d Host1:%s Host2:%s\n",
				joinResponse->nodeCount,
				joinResponse->nodeInformation[0].hostName,
				joinResponse->nodeInformation[1].hostName);
	}

	/* Create new TCP connections to those new nodes */
	for(ix=0 ; ix<joinResponse->nodeCount ; ix++)
	{
		nodeId = joinResponse->nodeInformation[ix].nodeId;

		/* Connect with each node */
		if((tcpSocket = connectToNewNode(joinResponse->nodeInformation[ix].hostName, joinResponse->nodeInformation[ix].tcpPortNumber)) == -1)
		{
			return -1;
		}

		/* Get lock before accessing node database */
		pthread_mutex_lock(&mutex_nodeDB);

		/* Store socket information in Node Database */
		nodeInformation[nodeId].tcpSocketFd = tcpSocket;												/* TCP Socket */

#ifdef DEBUG
		printf("DEBUG: sendJoinReq, NodeId:%d Socket:%d\n", nodeId, nodeInformation[nodeId].tcpSocketFd);
#endif // DEBUG

		nodeInformation[nodeId].nodeId	=	nodeId;														/* NodeId */
		strcpy(nodeInformation[nodeId].hostName, joinResponse->nodeInformation[ix].hostName);			/* HostName */
		strcpy(nodeInformation[nodeId].tcpPortNumber, joinResponse->nodeInformation[ix].tcpPortNumber);	/* TCP Port */
		strcpy(nodeInformation[nodeId].udpPortNumber, joinResponse->nodeInformation[ix].udpPortNumber);	/* UDP Port */

		/* Release lock after accessing node database */
		pthread_mutex_unlock(&mutex_nodeDB);
	}

	/* Signal to TCP thread to start listening to incomming data on TCP connection */
	if((rv = sem_post(&sem_startTcpConListen)) == -1)
	{
		printf("ERROR: sendJoinReq, semaphore failed while posting, ECODE:%s\n",
				gai_strerror(rv));
		exit(1);
	}

	/* Clear the resources */
	freeaddrinfo(servinfo);
	//close(udpSockfd);

	return 0;
}

/****************************************************************************************
 /** connectToNewNode: Function to establish TCP connection with nodes
  *
  * @param[in] hostName: 	Host name of the new node
  *
  * @param[in] tcpPort: 	TCP Port number of the new node
  *
 ****************************************************************************************/
int connectToNewNode(char *hostName, char *TcpPortNumber)
{
	int returngetaddrinfo;
	int rv;
	struct addrinfo tempSocketInfo, *finalSocketInfo, *iSocketLoopIter;
	int socketfd;

	(void) memset(&tempSocketInfo, 0, sizeof tempSocketInfo); 	/* Remember this, this COSTED you much */
	tempSocketInfo.ai_family 	=   AF_UNSPEC; 					/* Don't care IPv4 or IPv6 */
	tempSocketInfo.ai_socktype 	=   SOCK_STREAM; 				/* TCP stream sockets */

	if((returngetaddrinfo = getaddrinfo(hostName, TcpPortNumber, &tempSocketInfo, &finalSocketInfo)) != 0)
	{
		printf("Error: connectToNewNode ECODE %s for %s Port %s\n", gai_strerror(returngetaddrinfo), hostName,
				TcpPortNumber);

		return -1;
	}

	/* Loop through all valid values return by getaddrinfo() */
	for(iSocketLoopIter = finalSocketInfo ; iSocketLoopIter != NULL ; iSocketLoopIter = iSocketLoopIter->ai_next)
	{
		if ((socketfd = socket(iSocketLoopIter->ai_family, iSocketLoopIter->ai_socktype, iSocketLoopIter->ai_protocol)) == -1)
		{
			perror("Error: Socket connection failed\n");
			continue;
		}

		if ((rv = connect(socketfd, iSocketLoopIter->ai_addr, iSocketLoopIter->ai_addrlen)) == -1)
		{
			close(socketfd);

			continue;
		}
		else
		{
			/* Connection established */
			printf("Connected to %s\n", hostName);
		}

		break;
	}

	freeaddrinfo(finalSocketInfo);

	return socketfd;
}

/****************************************************************************************
 /** sendConnectReq: Function to send connect request to m nodes
  *
  * @param[in] toNodeId: The node ID of the node which is the recipient of ConnectReq message
  *
  * @param[in] ownNodeId: Own node Id
  *
  * @return 0 if PASSED or -1 if FAILED
 *
 ****************************************************************************************/
int sendConnectReq(int toNodeId, int ownNodeId)
{
	mConnectRequest connectRequest;
	char sendBuffer[200];
	int tcpSocketFd;
	int sentBytes = 0;

	/* Form the message */
	connectRequest.messageId = ConnectionRequest;
	connectRequest.nodeInformation.nodeId = ownNodeId;
	strcpy(connectRequest.nodeInformation.hostName, nodeInformation[ownNodeId].hostName);
	strcpy(connectRequest.nodeInformation.tcpPortNumber, nodeInformation[ownNodeId].tcpPortNumber);
	strcpy(connectRequest.nodeInformation.udpPortNumber, nodeInformation[ownNodeId].udpPortNumber);

	tcpSocketFd = nodeInformation[toNodeId].tcpSocketFd;

	sentBytes = sizeof(connectRequest);

	/* Clear sendBuffer before using it */
	memset(sendBuffer,0, sizeof(sendBuffer));

	/* Copy the date to be transmitted */
	memcpy(sendBuffer, &connectRequest, sentBytes);

	if(sendDataOnTCP(tcpSocketFd, sendBuffer, &sentBytes) == -1)
	{
		printf("ERROR: sendConnectReq, Can't send ConnectReq, (%d/%d) bytes sent\n",
				sentBytes/sizeof(connectRequest));

		return -1;
	}
	else
	{
		printf("INFO: ConnectReq sent to NodeId:%d host:%s\n",
				toNodeId,
				nodeInformation[toNodeId].hostName);
	}

	return 0;
}

/****************************************************************************************
 /** sendRouteUpdate: Function to send RouteUpdate to immediate neighbors
  *
  * @param[in] toNodeId: The node ID of the node which is the recipient of RouteUpdate message
  *
  * @param[in] ownNodeId: Own node Id
  *
  * @param[in] originatorNode: This indicates if this node is the originator of the update
  *                            -1: Originator node
  *                            else node is formwarding the message
  *
  * @return 0 if PASSED or -1 if FAILED
 *
 ****************************************************************************************/
int sendRouteUpdate(int toNodeId, int ownNodeId, int originatorNode)
{
	mRouteInformation routeInformation;
	int tcpSocketFd;
	int sendBytes;
	char sendBuffer[200];

#ifdef DEBUG
	/* DEBUG */
	printf("Before memcpy\n");
	printf("DEBUG: Distance Vector: ");
	for(int ix=0 ; ix<MAX_NUMBER_OF_NODES ; ix++)
	{
		printf("%d ",DistV[ix]);
	}
	printf("\n");

	printf("DEBUG: Degree Vector: ");
	for(int ix=0 ; ix<MAX_NUMBER_OF_NODES ; ix++)
	{
		printf("%d ",DegV[ix]);
	}
	printf("\n");
#endif // DEBUG

	/* Prepare the message */
	routeInformation.messageId 	= RouteInformation;
	routeInformation.nodeId 	= ownNodeId;
	memcpy(routeInformation.newDegV, DegV, sizeof(DegV));
	memcpy(routeInformation.newDistV, DistV, sizeof(DistV));

	if(originatorNode == -1)
	{
		/* This node is the originator, hence own node information */
		routeInformation.nodeInformation.nodeId = ownNodeId;
		strcpy(routeInformation.nodeInformation.hostName,nodeInformation[ownNodeId].hostName);
		strcpy(routeInformation.nodeInformation.tcpPortNumber, nodeInformation[ownNodeId].tcpPortNumber);
		strcpy(routeInformation.nodeInformation.udpPortNumber, nodeInformation[ownNodeId].udpPortNumber);
	}
	else
	{
		/* Forwarding the route information from some other node, fill its information */
		routeInformation.nodeInformation.nodeId = originatorNode;
		strcpy(routeInformation.nodeInformation.hostName,nodeInformation[originatorNode].hostName);
		strcpy(routeInformation.nodeInformation.tcpPortNumber, nodeInformation[originatorNode].tcpPortNumber);
		strcpy(routeInformation.nodeInformation.udpPortNumber, nodeInformation[originatorNode].udpPortNumber);
	}

#ifdef DEBUG
	/* DEBUG */
	printf("DEBUG: Distance Vector: ");
	for(int ix=0 ; ix<MAX_NUMBER_OF_NODES ; ix++)
	{
		printf("Orig:%d Copy:%d\n",
				DistV[ix],
				routeInformation.newDistV[ix]);
	}
	printf("\n");

	printf("DEBUG: Before sending RouteUpdate Degree Vector: ");
	for(int ix=0 ; ix<MAX_NUMBER_OF_NODES ; ix++)
	{
		printf("%d ",DegV[ix]);
	}
	printf("\n");
#endif // DEBUG


	/* Get the socket fd from database */
	tcpSocketFd = nodeInformation[toNodeId].tcpSocketFd;

	/* Clear data */
	memset(sendBuffer, 0, sizeof(sendBuffer));

	sendBytes = sizeof(routeInformation);

	memcpy(sendBuffer, &routeInformation, sendBytes);

	if(sendDataOnTCP(tcpSocketFd, sendBuffer, &sendBytes) == -1)
	{
		printf("ERROR: sendRouteUpdate, Failure in sending RouteInformaton, nodeId:%d (%d/%d) bytes sent\n",
				toNodeId, sendBytes, sizeof(routeInformation));

		printNodeDB();

		return -1;
	}
	else
	{
		printf("DEBUG: RouteInformation sent to Host:%s\n", nodeInformation[toNodeId].hostName);
	}

	return 0;
}
