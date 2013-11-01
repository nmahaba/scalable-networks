
/*
 * FileUtilities.c
 *
 *  Created on: Sep 21, 2013
 *      Author: Niranjan
 */

#include <FileUtilities.h>

/* EXTERN declarations - START 	*/
extern SNodeInformation nodeInformation[MAX_NUMBER_OF_NODES];
/* EXTERN declarations - END 	*/

/****************************************************************************************
 /** initializeNodeDB: Function to read the configuration file and build node database
  *
  * @param[in] connectionsFile: The name of the file which has information about all nodes
  * 							in the system
  *
  * @return 0 if PASSED or -1 if FAILED
 *
 ****************************************************************************************/
int initializeNodeDB(char *connectionsFile)
{
	FILE *fd_connectionsFile;
	char line[100];
	char connectionFields[4][50];
	char *token;
	int index 		= 0;
	int nodeIndex 	= 0;

	/* File name validation */
	if(connectionsFile == NULL)
	{
		printf("ERROR: Connections file name is NULL\n");
		return -1;
	}

	/* Open the file in read mode */
	fd_connectionsFile = fopen(connectionsFile,"r");

	if(fd_connectionsFile == NULL)
	{
		printf("ERROR: Cannot open connections file: %s\nErrorDesc: %s",
				connectionsFile, strerror(errno));
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

	while(fgets(line, 100, fd_connectionsFile) != NULL)
	{
		//printf("%s", line);

		token = strtok(line," ");

		index = 0;
		while(token != NULL)
		{
			strcpy(connectionFields[index++], token);
			token = strtok(NULL," ");
		}

		/* Store the information in the database */
		strcpy(nodeInformation[nodeIndex].nodeId, connectionFields[0]);		/* NodeId */
		strcpy(nodeInformation[nodeIndex].hostName, connectionFields[1]);	/* HostName */
		strcpy(nodeInformation[nodeIndex].tcpPortNumber, connectionFields[2]);	/* TCP Port */
		strcpy(nodeInformation[nodeIndex].udpPortNumber, connectionFields[3]);	/* UDP Port */

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

