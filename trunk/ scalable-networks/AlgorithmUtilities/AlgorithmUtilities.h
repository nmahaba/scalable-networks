/*
 * AlgorithmUtilities.h
 *
 *  Created on: Nov 27, 2013
 *      Author: Niranjan
 */

#ifndef ALGORITHMUTILITIES_H_
#define ALGORITHMUTILITIES_H_

#include <Constants.h>
#include <SRouteInformation.h>
#include <SBfsQNode.h>
#include <vector>
#include <algorithm>
#include <stdio.h>

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
int updateDVM(mRouteInformation routeInformation);

/****************************************************************************************
  * updateDVM: Function to read the node configuration file and build node database
  *
  * @param[in] startNode: The nodeId from where BFS should start
  *
  * @param[in] ownNodeId: Own node id
  *
  * PS: It access the below global variables
  * 	1. AdjList
  * 	2. DVM
  * 	3. DistV
  * 	4. DegV
  *
  * @return void
  *
 ****************************************************************************************/
void BFS(int startNode, int ownNodeId);

#endif /* ALGORITHMUTILITIES_H_ */
