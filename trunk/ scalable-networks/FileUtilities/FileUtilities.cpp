
/*
 * FileUtilities.c
 *
 *  Created on: Sep 21, 2013
 *      Author: Niranjan
 */

#include<FileUtilities.h>

/* EXTERN declarations - START 	*/
extern SNodeInformation nodeInformation[MAX_NUMBER_OF_NODES];
extern int connectionInfo[MAX_NUMBER_OF_NODES];
extern int ownNodeId;
/* EXTERN declarations - END 	*/

/****************************************************************************************
 /** initializeNodeDB: Function to read the node configuration file and build node database
  *
  * @param[in] connectionsFile: The name of the file which has information about all nodes
  * 							in the system
  *
  * @return 0 if PASSED or -1 if FAILED
 *
 ****************************************************************************************/
int initializeNodeDB(char *nodeInfoFile)
{
	FILE *fd_nodeInfoFile;
	char line[100];
	char nodeFields[4][50];
	char *token;
	int index 		= 0;
	int nodeIndex 	= 0;

	/* File name validation */
	if(nodeInfoFile == NULL)
	{
		printf("ERROR: Node information file name is NULL\n");
		return -1;
	}

	/* Open the file in read mode */
	fd_nodeInfoFile = fopen(nodeInfoFile,"r");

	if(fd_nodeInfoFile == NULL)
	{
		printf("ERROR: Cannot open node information file: %s\nErrorDesc: %s",
				nodeInfoFile, strerror(errno));
		return -1;
	}

	/* Format of the configuration file
	 * <NodeId> <HostName> <TCPPortNumber> <UDPPortNumber>
	 */

	/* Below is done in the below segment
	 * 1. Read the file one line at a time.
	 * 2. Split the line on spaces and extract the 4 required fields.
	 * 3. Store the fields in the database.
	 * */

	/* Index to the node database */
	nodeIndex = 0;

	while(fgets(line, 100, fd_nodeInfoFile) != NULL)
	{
		//printf("%s", line);

		token = strtok(line," ");

		index = 0;
		while(token != NULL)
		{
			strcpy(nodeFields[index++], token);
			token = strtok(NULL," ");
		}

		/* Store the information in the database */
		strcpy(nodeInformation[nodeIndex].nodeId, nodeFields[0]);		/* NodeId */
		strcpy(nodeInformation[nodeIndex].hostName, nodeFields[1]);	/* HostName */
		strcpy(nodeInformation[nodeIndex].tcpPortNumber, nodeFields[2]);	/* TCP Port */
		strcpy(nodeInformation[nodeIndex].udpPortNumber, nodeFields[3]);	/* UDP Port */

		nodeInformation[nodeIndex].tcpSocketFd = -1;
		nodeInformation[nodeIndex].udpSocketFd = -1;

		/* Increment nodeIndex for next entry */
		nodeIndex++;
	}

	/* For the remaining entries fill NULL entries */
	for(;nodeIndex<MAX_NUMBER_OF_NODES ; nodeIndex++)
	{
		strcpy(nodeInformation[nodeIndex].nodeId, "");		/* NodeId */
		strcpy(nodeInformation[nodeIndex].hostName, "");	/* HostName */
		strcpy(nodeInformation[nodeIndex].tcpPortNumber, "");	/* TCP Port */
		strcpy(nodeInformation[nodeIndex].udpPortNumber, "");	/* UDP Port */

		nodeInformation[nodeIndex].tcpSocketFd = -1;
		nodeInformation[nodeIndex].udpSocketFd = -1;
	}

	return 0;
}

/****************************************************************************************
 /** readConnectionsFile: Function to read the connection configuration file and build node database
  *
  * @param[in] connectionsFile: The name of the file which has information about all connections
  * 							in the system
  *
  * @return 0 if PASSED or -1 if FAILED
 *
 ****************************************************************************************/
int readConnectionsFile(char *connectionsInfoFile)
{
	FILE *fd_connectionsInfoFile;
	char line[100];
	char connectionFields[2][4];	/* Stores sourceId and destinationId */
	char *token;
	int index 				= 0;
	int nodeIndex 			= 0;
	int sourceNodeId 		= 0;
	int destinationNodeId 	= 0;

	/* File name validation */
	if(connectionsInfoFile == NULL)
	{
		printf("ERROR: Connections file name is NULL\n");
		return -1;
	}

	/* Open the file in read mode */
	fd_connectionsInfoFile = fopen(connectionsInfoFile,"r");

	if(fd_connectionsInfoFile == NULL)
	{
		printf("ERROR: Cannot open connections file: %s\nErrorDesc: %s",
				connectionsInfoFile, strerror(errno));
		return -1;
	}

	/*
	 * Format of connections file
	 * <Node1>,<Node2>
	 */

	/* Below is done in the below segment
	 * 1. Read the file one line at a time.
	 * 2. Split the line on "," and extract the source and destination nodes.
	 * 3. Store the connection information in the database.
	 * */

	/* Index to the node database */
	nodeIndex = 0;

	while(fgets(line, 100, fd_connectionsInfoFile) != NULL)
	{
		//printf("%s", line);

		token = strtok(line,",");

		index = 0;
		while(token != NULL)
		{
			strcpy(connectionFields[index++], token);
			token = strtok(NULL,",");
		}

		/* Get the source and destination nodeId
		 * Convert to integer
		 * set that corresponding position to 1
		 * This shows that this node has a connection
		 * with every node with a 1 set in connectionInfo array
		 * Eg: 	For node 1 we have
		 * 		connectionInfo[2] = 1
		 * 		connectionInfo[3] = 1
		 * 		connectionInfo[4] = 1
		 * 		This means that node 1 has a connection with 2,3,4
		 */

		sourceNodeId 		= atoi(connectionFields[0]);
		destinationNodeId	= atoi(connectionFields[1]);

		/* Store the information */
		if(sourceNodeId == ownNodeId)
		{
			connectionInfo[destinationNodeId] = 1;
		}
		else if(destinationNodeId == ownNodeId)
		{
			connectionInfo[sourceNodeId] = 1;
		}
	}

	return 0;
}

