/*
 * Main.c
 *
 *  Created on: Nov 1, 2013
 *      Author: Niranjan
 */

#include<iostream>
#include<FileUtilities.h>
#include<Constants.h>

using namespace std;


/* Global variables - Start */
SNodeInformation nodeInformation[MAX_NUMBER_OF_NODES];
/* Global variables - End   */

/* Debug functions */
void printNodeDB()
{
	int index = 0;

	printf("************************************\n");
	printf("NodeId\tHostName\tTCPPort\tUDPPort\n");
	printf("************************************\n");

	for(index=0 ; index<MAX_NUMBER_OF_NODES ; index++)
	{
		printf("%s %15s %10s %10s\n",
				nodeInformation[index].nodeId,
				nodeInformation[index].hostName,
				nodeInformation[index].tcpPortNumber,
				nodeInformation[index].udpPortNumber);
	}
	printf("************************************\n");
}


/* Command line arguments
 * nodes.out connectionsFile */
int main(int argc, char **argv)
{
	/* Check command line arguments */
	if(argc != 2)
	{
		printf("ERROR: Invalid command line arguments\nnodes.out <<ConnectiosnFile>>\n");
		return -1;
	}

	/* Read the connections file and fill the database */
	if(initializeNodeDB(argv[1]) == -1)
	{
		return -1;
	}

#if 0
	/* Debug function */
	printNodeDB();
#endif

	return 0;
}





