
/*
 * FileUtilities.c
 *
 *  Created on: Sep 21, 2013
 *      Author: Niranjan
 */

#include<FileUtilities.h>

using namespace std;

/* EXTERN declarations - START 	*/
extern SNodeInformation nodeInformation[MAX_NUMBER_OF_NODES];
extern int connectionInfo[MAX_NUMBER_OF_NODES];
extern int DVM[MAX_NUMBER_OF_NODES][MAX_NUMBER_OF_NODES]; 	/* The distance vector matrix */
extern int DistV[MAX_NUMBER_OF_NODES];					 	/* The distance vector */
extern int DegV[MAX_NUMBER_OF_NODES];						/* The degree vector */
extern vector< vector<int> > AdjList;  						/* Used initially for calculating the DegV */
extern pthread_mutex_t mutex_nodeDB;
extern int numOfPrimeNodes;
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
	int index 					= 0;
	int length 					= 0;

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

	/* Get lock for NodeDB */
	pthread_mutex_lock(&mutex_nodeDB);

	/* Initialize the database */
	for(nodeId=0 ; nodeId<MAX_NUMBER_OF_NODES ; nodeId++)
	{
		nodeInformation[nodeId].nodeId = -1;					/* NodeId */
		strcpy(nodeInformation[nodeId].hostName, "");	/* HostName */
		strcpy(nodeInformation[nodeId].tcpPortNumber, "");	/* TCP Port */
		strcpy(nodeInformation[nodeId].udpPortNumber, "");	/* UDP Port */

		nodeInformation[nodeId].tcpSocketFd = -1;
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
		/* Count number of prime nodes */
		numOfPrimeNodes++;

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
		nodeInformation[nodeId].nodeId = atoi(nodeFields[0]);			/* NodeId */
		strcpy(nodeInformation[nodeId].hostName, nodeFields[1]);		/* HostName */
		strcpy(nodeInformation[nodeId].tcpPortNumber, nodeFields[2]);	/* TCP Port */

		/* UDP port had newline character with it - Problem, to solve it remove the newline character */
		nodeFields[3][strlen(nodeFields[3])-1] = '\0';

		strcpy(nodeInformation[nodeId].udpPortNumber, nodeFields[3]);	/* UDP Port */

		nodeInformation[nodeId].tcpSocketFd = -1;
	}
	
	fclose(fd_nodeInfoFile);

	/* Release lock for NodeDB */
	pthread_mutex_unlock(&mutex_nodeDB);

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
	
	fclose(fd_connectionsInfoFile);

	return 0;
}

/****************************************************************************************
  * fillAlgorithmDB: Function to read the connectionFile and fill DVM, DisV and DegV
  *
  * @param[in]: connectionInfoFile: File name of connection Info file
  *
  * @param[in]: ownNodeId: Own node Id
  *
  * @return 0 if success or -1 if failure
  *
 ****************************************************************************************/
int fillAlgorithmDB(char *connectionsInfoFile, int ownNodeId)
{
	/* Should perform BFS one each of the neighbors, keep a list of your neighbors */
	vector<int>myNbor;	//Should perform BFS on each node

	/* Fill up the DistV with all infinity */
	std::fill_n(DistV, MAX_NUMBER_OF_NODES, INFINITY);

	/* Initialize DegV to 0 */
	std::fill_n(DegV, MAX_NUMBER_OF_NODES, 0);

	DistV[ownNodeId] = 0 ;

	/* Fill up the DVM with all infinity */
	for(int i = 0 ; i < MAX_NUMBER_OF_NODES ; i++)
	{
		for(int j = 0 ; j < MAX_NUMBER_OF_NODES ; j++)
		{
			DVM[i][j] = INFINITY;
		}
	}

	/* DEBUG */
	printf("fillAlgorithmDBy\n");
	printf("DEBUG: Distance Vector: ");
	for(int ix=0 ; ix<MAX_NUMBER_OF_NODES ; ix++)
	{
		printf("%d ",DistV[ix]);
	}
	printf("\n");

	printf("DEBUG: Degree Vector: ");
	for(int ix=0 ; ix<MAX_NUMBER_OF_NODES ; ix++)
	{
		printf("%d ",DegV[ix]);
	}
	printf("\n");

	DVM[ownNodeId][ownNodeId] = 0 ;

#ifdef DEBUG
	cout<<AdjList.size()<<endl;
#endif // DEBUG

	/* Read Contents of the Connection Info File into the Adjacency List */
	ifstream conFile(connectionsInfoFile);

	/* Error checking */
	if(conFile.is_open() == -1)
	{
		printf("ERROR: fillAlgorithmDB, ConnectionFile open failed, ECODE:%s\n", strerror(errno));
		return -1;
	}

	while(!conFile.eof())
	{
		/* Read and parse the connectionInfoFile Line */
		string line;

		if(!(conFile>>line))
		{
			break;
		}

		char* inputs 	= strtok((char*)line.c_str(),",");
		int left   		= atoi(inputs);

		inputs = strtok(NULL,",");
		int right = atoi(inputs);

		/*Fill up the adjacency list with left neighbor as right and vice versa*/
		AdjList.at(left).push_back(right);
		AdjList.at(right).push_back(left);

		/* Check if my neighbor and add to the myNbor */
		if(left == ownNodeId)
		{
			myNbor.push_back(right);
		}
		else if(right == ownNodeId)
		{
			myNbor.push_back(left);
		}
	}

#ifdef DEBUG
	/* Print out adjacency list */
	for(int i = 0 ; i < AdjList.size() ; i++)
	{
		cout<<" Neighbors of  "<<i<<" : "<<endl;

		for(int j = 0 ; j < AdjList.at(i).size() ; j++)
			cout<<AdjList.at(i).at(j)<<" ";

		cout<<endl;

	}
#endif

	/* Since we now have the adjacency list filled up we can initialize the degree vector */
	for(int i = 0 ; i < AdjList.size() ; i++)
	{
		DegV[i] = AdjList.at(i).size() ;
	}

	/* Now perform BFS on each element of myNeigh
	 * Presumably the members of myNeigh are at level 1
	 * Everyone after that is at level i + 1
	 */
	for(int i=0 ; i<myNbor.size() ; i++)
	{
		/* Call BFS on the specific Neighbor */
		BFS(myNbor.at(i), ownNodeId);
	}

#ifdef DEBUG
	cout << "Final DVM:" << endl ;
	for(int i = 0 ; i < MAX_NUMBER_OF_NODES ; i++)
	{
		for(int j = 0 ; j < MAX_NUMBER_OF_NODES ; j++)
		{
			cout<<DVM[i][j]<<"\t" ;
		}
		cout<<endl;
	}

	cout << "Final Degree Vector: " << endl ;
	for(int i = 0 ; i < MAX_NUMBER_OF_NODES ; i++)
	{
		cout<<DegV[i]<<"\t";
	}
	cout<<endl;

	cout << "Final Distance Vector: " << endl ;
	for(int i = 0 ; i < MAX_NUMBER_OF_NODES ; i++)
	{
		cout<<DistV[i]<<"\t";
	}
	cout<<endl;

#endif

	return 0;
}
