/*
 * FileUtilities.h
 *
 *  Created on: Sep 21, 2013
 *      Author: Niranjan
 */

#ifndef FILE_UTILITIES_H_
#define FILE_UTILITIES_H_

#include<stdio.h>
#include<string.h>
#include<dirent.h>
#include<sys/stat.h>
#include<unistd.h>
#include<errno.h>
#include<Constants.h>
#include<SNodeInformation.h>
#include<stdlib.h>

/****************************************************************************************
 /** initializeNodeDB: Function to read the node configuration file and build node database
  *
  * @param[in] connectionsFile: The name of the file which has information about all nodes
  * 							in the system
  *
  * @return 0 if PASSED or -1 if FAILED
 *
 ****************************************************************************************/
int initializeNodeDB(char *nodeInfoFile);

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
int readConnectionsFile(char *connectionsInfoFile, int ownNodeId);

#endif /* FILE_UTILITIES_H_ */

