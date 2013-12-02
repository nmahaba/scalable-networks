/*
 * AlgorithmUtilities.cpp
 *
 *  Created on: Nov 27, 2013
 *      Author: Niranjan
 */

#include <AlgorithmUtilities.h>

using namespace std;

/* EXTERN declarations - START 	*/
extern int DVM[MAX_NUMBER_OF_NODES][MAX_NUMBER_OF_NODES]; 	/* The distance vector matrix */
extern int DistV[MAX_NUMBER_OF_NODES];					 	/* The distance vector */
extern int DegV[MAX_NUMBER_OF_NODES];						/* The degree vector */
extern vector< vector<int> > AdjList;/* Used initially for calculating the DegV */
/* EXTERN declarations - END 	*/

/****************************************************************************************
  * updateDVM: Function to read the node configuration file and build node database
  *
  * @param[in] routeInformation: The route information which has the updates.
  *
  * @param[in] eRouteType: This specifies what kind of update is this, a routine or
  *                        a new node update
  *
  * @param[in] ownNodeId: Id of own node
  *
  * @return
  * Return the type of outcome
  * DV       DegV    retVal
  * 0        0        0 - No change in either
  * 0        1        1 - Change in DegV only
  * 1        0        2 - Change in DV only
  * 1        1        3 - Change in both DegV and DV
  *
 ****************************************************************************************/
int updateDVM(mRouteInformation routeInformation, eRouteType updateType, int ownNodeId)
{
	int degv_Val = 0 ;
	int dv_Val = 0 ;

	/* Check for updates with the degree vectors */
	for(int i = 0 ; i < MAX_NUMBER_OF_NODES ; i++)
	{
		if(DegV[i] < routeInformation.newDegV[i])
		{
			degv_Val 	= 1 ;
			DegV[i] 	= routeInformation.newDegV[i];
		}
	}

	/* Update own node degree if this is a node join */
	if(updateType == newNodeJoinUpdate)
	{
		DegV[ownNodeId] += 1;
	}

	/* Check for updates with the distance vectors */
	for(int i = 0 ; i < MAX_NUMBER_OF_NODES ; i++)
	{
		DVM[routeInformation.nodeId][i] = routeInformation.newDistV[i] + 1;

		if(DistV[i] > (routeInformation.newDistV[i] + 1))
		{
			dv_Val 		= 2;
			DistV[i] 	= routeInformation.newDistV[i] + 1;
		}
	}

	return degv_Val + dv_Val ;
}

/****************************************************************************************
  * BFS: Function to read the node configuration file and build node database
  *
  * @param[in] startNode: The nodeId from where BFS should start
  *
  * @param[in] ownNodeId: Own node id
  *
  * @return void
  *
 ****************************************************************************************/
void BFS(int startNode, int ownNodeId)
{
	int DV_i[MAX_NUMBER_OF_NODES];						// Distance Vector of the neighbor node i
	std::fill_n(DV_i, MAX_NUMBER_OF_NODES, INFINITY);	// fill up the node i's distance vector with infinity
	int visited[MAX_NUMBER_OF_NODES];					// All are initially set to zero
	vector<BFSQNode> toVisit;							// A queue that lists out nodes to visit next

	/* Initialize visited to 0 */
	std::fill_n(visited, MAX_NUMBER_OF_NODES, 0);

	visited[startNode] 			= 1;
	DVM[startNode][startNode] 	= 1;
	DVM[startNode][ownNodeId] 	= 1;
	DistV[startNode] 			= 1;

	/* Debug */
#if DEBUG
	printf("BFS: OwnNodeId:%d StartNode:%d\n",ownNodeId, startNode);
	printf("Adjacency List:\n");
	for(int ix=0 ; ix<AdjList.size() ; ix++)
	{
		for(int jx=0 ; jx<AdjList.at(ix).size() ; jx++)
		{
			printf("%d ", AdjList.at(ix).at(jx));
		}
		printf("\n");
	}
#endif

	/* Fill up the Queue with nodes that are next to visit */
	for(int i = 0 ; i < AdjList.at(startNode).size() ; i++)
	{
		int nextNode = AdjList.at(startNode).at(i);

		/* Push the node as a to visit node to the queue */
		BFSQNode node ;
		node.level 	= 2 ;
		node.nodeId = nextNode ;
		toVisit.push_back(node);
	}

#ifdef DEBUG
	printf("DEBUG: To be visited:");
	/* Debug */
	for(int i = 0 ; i < toVisit.size() ; i++)
	{
		printf("%d ", toVisit.at(i).nodeId);
	}
	printf("\n");
#endif // DEBUG

	/* The BFS Algorithm */
	while(toVisit.size() != 0)
	{
		/* Visit the node at the front of the queue */
		BFSQNode curNode = toVisit.front();

		/* See if already visited */
		if(visited[curNode.nodeId] == 1)
		{
			toVisit.erase(toVisit.begin());

#ifdef DEBUG
			printf("DEBUG: NodeId:%d already visited\n", curNode.nodeId);
#endif // DEBUG

			continue;
		}

		/* Visit this node */
		visited[curNode.nodeId] 		= 1;

#ifdef DEBUG
		printf("DEBUG: Visiting NodeId:%d\n", curNode.nodeId);
#endif // DEBUG

		DVM[startNode][curNode.nodeId] 	= curNode.level;

		/* Do an update check on the DistV */
		if(DistV[curNode.nodeId] > curNode.level)
		{
			DistV[curNode.nodeId] 		= curNode.level;

#ifdef DEBUG
			printf("Changing route, NodeId:%d Level:%d\n", curNode.nodeId, curNode.level);
#endif // DEBUG

		}

		/* Add all of it's unvisited neighbors to the queue */
		for(int i=0 ; i<AdjList.at(curNode.nodeId).size() ; i++)
		{
			int nextNode = AdjList.at(curNode.nodeId).at(i);

			if(visited[nextNode] != 1)
			{
				BFSQNode next;

				next.level 	= curNode.level + 1;
				next.nodeId = nextNode;

				toVisit.push_back(next);
			}
		}

		/* Remove the node itself */
		toVisit.erase(toVisit.begin());
	}
}


