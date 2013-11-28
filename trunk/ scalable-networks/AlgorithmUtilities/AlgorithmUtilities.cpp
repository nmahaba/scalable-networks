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
  * @return
  * Return the type of outcome
  * DV       DegV    retVal
  * 0        0        0 - No change in either
  * 0        1        1 - Change in DegV only
  * 1        0        2 - Change in DV only
  * 1        1        3 - Change in both DegV and DV
  *
 ****************************************************************************************/
int updateDVM(mRouteInformation routeInformation)
{
	int degv_Val = 0 ;
	int dv_Val = 0 ;

	/* Check for updates with the degree vectors */
	for(int i = 0 ; i < MAX_NUMBER_OF_NODES ; i++)
	{
		if(DegV[i] < routeInformation.newDegV[i])
		{
			degv_Val 	= 1 ;
			DegV[i] 	= routeInformation.newDegV[i] ;
		}
	}

	/* Check for updates with the distance vectors */
	for(int i = 0 ; i < MAX_NUMBER_OF_NODES ; i++)
	{
		DVM[routeInformation.nodeId][i] = routeInformation.newDegV[i] + 1 ;

		if(DistV[i] > (routeInformation.newDegV[i] + 1))
		{
			dv_Val 		= 2 ;
			DistV[i] 	= routeInformation.newDegV[i] + 1 ;
		}
	}

	return degv_Val + dv_Val ;
}

/****************************************************************************************
  * updateDVM: Function to read the node configuration file and build node database
  *
  * @param[in] startNode: The nodeId from where BFS should start
  *
  * @param[in] ownNodeId: Own node id
  * @return void
  *
 ****************************************************************************************/
void BFS(int startNode, int ownNodeId)
{
	int DV_i[MAX_NUMBER_OF_NODES];						// Distance Vector of the neighbor node i
	std::fill_n(DV_i, MAX_NUMBER_OF_NODES, INFINITY);	// fill up the node i's distance vector with infinity
	int visited[MAX_NUMBER_OF_NODES];					// All are initially set to zero
	vector<BFSQNode> toVisit;							// A queue that lists out nodes to visit next

	visited[ownNodeId] = 1 ;
	visited[startNode] = 1 ;
	DVM[startNode][startNode] = 1 ;
	DVM[startNode][ownNodeId] = 1 ;
	DistV[startNode] = 1 ;

	/* Fill up the Queue with nodes that are next to visit */
	for(int i = 0 ; i < AdjList.at(startNode).size() ; i++)
	{
		int nextNode = AdjList.at(startNode).at(i);
		if(nextNode != ownNodeId)
		{
			/* Push the node as a to visit node to the queue*/
			BFSQNode node ;
			node.level = 2 ;
			node.nodeId = nextNode ;
			toVisit.push_back(node);
		}
	}

	/* The BFS Algorithm */
	while(toVisit.size() != 0)
	{
		/* Visit the node at the front of the queue */
		BFSQNode curNode = toVisit.front();
		/* see if already visited */
		if(visited[curNode.nodeId] == 1)
		{

			toVisit.erase(toVisit.begin());
			continue;

		}

		/* Visit this node */
		visited[curNode.nodeId] = 1 ;
		DVM[startNode][curNode.nodeId] = curNode.level ;

		/* Do an update check on the DistV */
		if(DistV[curNode.nodeId] > curNode.level)
		{
			DistV[curNode.nodeId] = curNode.level ;
		}
		/* Add all of it's unvisited neighbors to the queue */
		for(int i = 0 ; i < AdjList.at(curNode.nodeId).size() ; i++)
		{
			int nextNode = AdjList.at(curNode.nodeId).at(i);
			if(visited[nextNode] != 1)
			{
				BFSQNode next;
				next.level = curNode.level + 1 ;
				next.nodeId = nextNode ;
				toVisit.push_back(next);
			}

		}
		/* Remove the node itself */
		toVisit.erase(toVisit.begin());
	}
}


