
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
	int nodeId;
	char *token;
	int index 		= 0;

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

	/* Initialize the database */
	for(nodeId=0 ; nodeId<MAX_NUMBER_OF_NODES ; nodeId++)
	{
		strcpy(nodeInformation[nodeId].nodeId, "");		/* NodeId */
		strcpy(nodeInformation[nodeId].hostName, "");	/* HostName */
		strcpy(nodeInformation[nodeId].tcpPortNumber, "");	/* TCP Port */
		strcpy(nodeInformation[nodeId].udpPortNumber, "");	/* UDP Port */

		nodeInformation[nodeId].tcpSocketFd = -1;
		nodeInformation[nodeId].udpSocketFd = -1;
	}

	/* Format of the configuration file
	 * <NodeId> <HostName> <TCPPortNumber> <UDPPortNumber>
	 */

	/* Below is done in the below segment
	 * 1. Read the file one line at a time.
	 * 2. Split the line on spaces and extract the 4 required fields.
	 * 3. Store the fields in the database.
	 * */

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

		/* PS: The contents of nodeFields array is below
		 * nodeFields[0] = nodeId
		 * nodeFields[1] = host name of the node
		 * nodeFields[2] = TCP Port number
		 * nodeFields[3] = UDP Port number */

		/* Extract the nodeId, this will be used to directly index the node database */
		nodeId = atoi(nodeFields[0]);

		/* Store the information in the database */
		strcpy(nodeInformation[nodeId].nodeId, nodeFields[0]);		/* NodeId */
		strcpy(nodeInformation[nodeId].hostName, nodeFields[1]);	/* HostName */
		strcpy(nodeInformation[nodeId].tcpPortNumber, nodeFields[2]);	/* TCP Port */
		strcpy(nodeInformation[nodeId].udpPortNumber, nodeFields[3]);	/* UDP Port */

		nodeInformation[nodeId].tcpSocketFd = -1;
		nodeInformation[nodeId].udpSocketFd = -1;
	}

	return 0;
}

/****************************************************************************************
 /** readConnectionsFile: Function to read the connection configuration file and build node database
  *
  * @param[in] connectionsFile: The name of the file which has information about all connections
  * 							in the system
  * @param[in] ownNodeId: 		Own NodeId
  *
  * @return 0 if PASSED or -1 if FAILED
 *
 ****************************************************************************************/
int readConnectionsFile(char *connectionsInfoFile, int ownNodeId)
{
	FILE *fd_connectionsInfoFile;
	char line[100];
	char connectionFields[2][4];	/* Stores sourceId and destinationId */
	char *token;
	int index 				= 0;
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

