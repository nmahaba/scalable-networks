#ifndef SOCKET_UTILITIES_H
#define SOCKET_UTILITIES_H

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <Constants.h>
#include <SNodeInformation.h>
#include <EMessageId.h>
#include <SConnectRequest.h>
#include <SConnectResponse.h>
#include <SRouteInformation.h>
#include <SJoinRequest.h>
#include <SJoinResponse.h>
#include <pthread.h>
#include <semaphore.h>

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
int sendDataOnTCP(int socketFd, char *sendBuffer, int *length);

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
int receiveDataOnTCP(int socketFd, char *receiveBuffer, int *length);

/****************************************************************************************
 /** connectToPrimeNodes: Function to establish connection with prime nodes using the
  * 						connection information
   * @param[in] ownNodeId Id of current node
 ****************************************************************************************/
int connectToPrimeNodes(int ownNodeId);

/****************************************************************************************
 /** processTCPConnections: Entry function which waits for new TCP connections or process
  * 						queries on TCP connections
  * @param[in] nodeId Id of the current node
 *
 ****************************************************************************************/
int processTCPConnections(int ownNodeId);

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
int createSocketAndBindAddress(int ownNodeId, int connectionType, int isServer, int toNodeId);

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
int updateScoketDescInNodeDB(struct sockaddr *pSockAddr, int fileDescriptor);

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
int sendJoinReq(int toNodeId, int ownNodeId, mJoinResponse* joinResponse);

/****************************************************************************************
 /** connectToNewNode: Function to establish TCP connection with nodes
  *
  * @param[in] hostName: 	Host name of the new node
  *
  * @param[in] tcpPort: 	TCP Port number of the new node
  *
 ****************************************************************************************/
int connectToNewNode(char *hostName, char *portNumber);


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
int sendConnectReq(int toNodeId, int ownNodeId);


/****************************************************************************************
 /** sendRouteUpdate: Function to send RouteUpdate to immediate neighbors
  *
  * @param[in] toNodeId: The node ID of the node which is the recipient of RouteUpdate message
  *
  * @param[in] ownNodeId: Own node Id
  *
  * @return 0 if PASSED or -1 if FAILED
 *
 ****************************************************************************************/
int sendRouteUpdate(int toNodeId, int ownNodeId);

#endif //SOCKET_UTILITIES_H
