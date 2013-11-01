/** /brief SNodesInformation.h
 *         This structure defines storage for node information, hostname and port number
 */

#ifndef SNODEINFORMATION
#define SNODEINFORMATION

/********************************************************************************
 * DEFINES - START SECTION
 ********************************************************************************/
#define MAXIMUM_CHARACTERS_IN_HOSTNAME			32
#define MAXIMUM_CHARACTERS_IN_FILENAME			32
#define IP_ADDRESS_LENGTH						32
#define MAXIMUM_PORT_LENGTH						16
#define MAXIMUM_NODE_ID_LENGTH					16
#define NO_VALUE								0xFFFF
#define TRUE									1
#define FALSE									0
/********************************************************************************
 * DEFINES - END SECTION
 ********************************************************************************/

typedef struct
{
	char 	nodeId[MAXIMUM_NODE_ID_LENGTH];				/**< Node ID of the node */
	char 	hostName[MAXIMUM_CHARACTERS_IN_HOSTNAME]; 	/**< Host name of the node */
	char 	tcpPortNumber[MAXIMUM_PORT_LENGTH];         /**< Port number of the node */
	char 	udpPortNumber[MAXIMUM_PORT_LENGTH];         /**< Port number of the node */
	int 	tcpSocketFd;								/**< TCP Socket file descriptor */
	int		udpSocketFd;								/**< UDP Socket file descriptor */
}SNodeInformation;

#endif // SNODEINFORMATION

