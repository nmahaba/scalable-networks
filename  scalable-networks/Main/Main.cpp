/*
 * Main.c
 *
 *  Created on: Nov 1, 2013
 *      Author: Niranjan
 */

#include<iostream>
#include<FileUtilities.h>
#include<Constants.h>
#include<stdlib.h>

using namespace std;


/* Global variables - Start */
SNodeInformation nodeInformation[MAX_NUMBER_OF_NODES];
int connectionInfo[MAX_NUMBER_OF_NODES] ={0};
int ownNodeId = -1;
/* Global variables - End   */

/* Debug functions */
void printNodeDB()
{
	int index = 0;

	printf("*****************************************\n");
	printf("NodeId\tHostName\tTCPPort\tUDPPort\n");
	printf("*****************************************\n");

	for(index=0 ; index<MAX_NUMBER_OF_NODES ; index++)
	{
		printf("%s %15s %10s %10s\n",
				nodeInformation[index].nodeId,
				nodeInformation[index].hostName,
				nodeInformation[index].tcpPortNumber,
				nodeInformation[index].udpPortNumber);
	}
}

/* Debug functions */
void printConnectionInfo()
{
	int index = 0;

	printf("*****************************************\n");
	printf("Connectivity information\n");
	printf("*****************************************\n");
	printf("Node %d is connected to\n", ownNodeId);

	for(index=0 ; index<MAX_NUMBER_OF_NODES ; index++)
	{
		if(connectionInfo[index] != 0)
		{
			printf("%d ",index);
		}
	}
	printf("\n");
}


/* Command line arguments
 * nodes.out nodeInformationFile  connectionInfoFile*/

/*
 * Main functionalities
 * 1. Read nodeInfo file and update node database
 * 2. Spawn a thread for handling UDP connections.
 * 3. First Mo nodes
 * 		a. Read connectionInfo file and update connection database.
 * 		b. Send TCP connections to other Mo nodes.
 * 4. Other non Mo nodes
 * 		a. Query information from other nodes.
 * 		b. Compute probability and send TCP connection request to m nodes
 * 		c. Update new network information.
 * 5. Handle new and existing TCP connections.
 */
int main(int argc, char **argv)
{
	/* Check command line arguments */
	if(argc != 4)
	{
		printf("ERROR: Invalid command line arguments\nnodes.out <<NodeId>> <<nodeInfoFile>> <<connectionInfoFile>>\n");
		return -1;
	}

	/* Store own node information */
	ownNodeId = atoi(argv[1]);

	/* Read the connections file and fill the database */
	if(initializeNodeDB(argv[2]) == -1)
	{
		return -1;
	}

#if 1
	/* Debug function */
	printNodeDB();
#endif

	/* Read connections file */
	if(readConnectionsFile(argv[3]) == -1)
	{
		return -1;
	}

#if 1
	/* Debug function */
	printConnectionInfo();
#endif

	return 0;
}





