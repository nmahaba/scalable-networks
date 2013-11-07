#include <SocketUtilities.h>

//#define DEBUG

/* EXTERN declarations - START 	*/
extern SNodeInformation nodeInformation[MAX_NUMBER_OF_NODES];
extern int connectionInfo[MAX_NUMBER_OF_NODES];
/* EXTERN declarations - END 	*/

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
			printf("ERROR: sendAll, send failed, ECODE %s\n", strerror(errno));
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
				printf("Error: connectToAllNodes ECODE %s for %s Port %s\n", strerror(returngetaddrinfo), nodeInformation[ix].hostName,
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

				if (connect(nodeInformation[ix].tcpSocketFd, iSocketLoopIter->ai_addr, iSocketLoopIter->ai_addrlen) == -1)
				{
					close(nodeInformation[ix].tcpSocketFd);

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

	socklen_t addrlen;
	struct sockaddr_storage remoteaddr; /* Client address */

	/* Create a socket, bind a address to the socket and get a file descriptor to listen to connections  */
	listener = createSocketAndBindAddress(ownNodeId);

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

	/* Add the listener to the master set */
	FD_SET(listener, &master);

	/* Keep track of the biggest file descriptor */
	fdmax = listener;

	/* Add the file descriptors obtained during connect() to the master list so that nodes can also listen to data sent by other nodes */
	for(ix=0 ; ix<MAX_NUMBER_OF_NODES ; ix++)
	{
		if(nodeInformation[ix].tcpSocketFd != -1)
		{
			FD_SET(nodeInformation[ix].tcpSocketFd, &master);
		}

		/* Update fdmax with the highest file descriptor in nodeInformation */
		if(nodeInformation[ix].tcpSocketFd > fdmax)
		{
			fdmax = nodeInformation[ix].tcpSocketFd;
		}
	}

	/* Store current node's listening file descriptor in nodeInformation in own NodeId's location */
	nodeInformation[ownNodeId].tcpSocketFd 	= listener;

	/* Main loop */
	while(true)
	{
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
						perror("Error: processTCPConnections, accept failed\n");
					}
					else
					{
						/*
						 * Update the new file descriptor in node's database which will used for future communication
						 */
						if(updateScoketDescInNodeDB((struct sockaddr *)&remoteaddr, newfd) == -1)
						{
							exit(1);
						}

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
					//printf("DEBUG: Data received by the node\n");

#if 0
					/* Receive data from one of the nodes */
					if ((numberOfBytesReceived = recv(i, &inMessage, sizeof inMessage, 0)) <= 0)
					{
						/* got error or connection closed by client */
						if (numberOfBytesReceived == 0)
						{
							/* Connection closed */
							printf("Error: processTCPConnections, node %d hung up\n", i);
						}
						else
						{
							perror("Error: processTCPConnections, recv FAILED\n");
						}

						close(i);

						FD_CLR(i, &master); /* Remove from Master file descriptor set */
					}
					else /* Valid data from a node */
					{
						printf("DEBUG: Data received by the node");
					}
#endif
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

	if(returnValue == -1)
	{
		perror("Error: updateFileDescriptorInNodeInformation, getnameinfo failed\n");
		return -1;
	}

	for(ix=0 ; ix<MAX_NUMBER_OF_NODES ; ix++)
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

	return 0;
}

/*************************************************************************
 /** createSocketAndBindAddress: Function to create a socket to listen and bind address
  *
  * @param[in] nodeId Id of the current node
  *
  * @return -1 if error
  *
 ************************************************************************/
int createSocketAndBindAddress(int nodeId)
{
	int listener;
	int reUsePort = 1;
	struct addrinfo tempSocketInfo, *finalSocketInfo, *iSocketLoopIter;

	(void) memset(&tempSocketInfo, 0, sizeof tempSocketInfo);

	tempSocketInfo.ai_family 	= AF_UNSPEC; 			/* Don't care IPv4 or IPv6 */
	tempSocketInfo.ai_socktype 	= SOCK_STREAM;	 		/* TCP stream sockets */
	tempSocketInfo.ai_flags 	= AI_PASSIVE;			/* Fill IP Address for you */

	if((getaddrinfo(NULL, nodeInformation[nodeId].tcpPortNumber, &tempSocketInfo, &finalSocketInfo)) != 0)
	{
		perror("ERROR: createSocketAndBindAddress, getaddrinfo failed\n");
		return -1;
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

		if (bind(listener, iSocketLoopIter->ai_addr, iSocketLoopIter->ai_addrlen) < 0)
		{
			close(listener);
			continue;
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

