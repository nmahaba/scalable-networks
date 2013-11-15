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
  * @param[in] nodeId Id of the current node
  *
  * @return -1 if error
  *
 ************************************************************************/
int createSocketAndBindAddress(int nodeId);

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

#endif //SOCKET_UTILITIES_H

