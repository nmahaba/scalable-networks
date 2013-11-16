/** /brief SNodesInformation.h
 *         This structure defines storage for node information, hostname and port number
 */

#ifndef SNODEINFORMATION
#define SNODEINFORMATION

#include <Constants.h>

typedef struct
{
	int 	nodeId;												/* Node ID of the node */
	char 	hostName[MAX_CHARACTERS_IN_HOSTNAME]; 				/* Host name of the node */
	char 	tcpPortNumber[MAX_CHARACTERS_IN_PORTNUMBER];        /* Port number of the node */
	char 	udpPortNumber[MAX_CHARACTERS_IN_PORTNUMBER];        /* Port number of the node */
	int 	tcpSocketFd;										/* TCP Socket file descriptor */
}SNodeInformation;

#endif // SNODEINFORMATION

