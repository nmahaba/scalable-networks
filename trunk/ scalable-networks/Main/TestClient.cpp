/*
** talker.c -- a datagram "client" demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define SERVERPORT "4950"    // the port users will be connecting to
#define MAXBUFLEN 1000
#define MAX_NUMBER_OF_NODES 10

int main(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numBytesSent;
    int numBytesReceived;
    char buf[MAXBUFLEN],sendbuf[MAXBUFLEN] = "QUERY";

    struct QMessage
    {
    	int nodeId;
    	int nodeDegree;
    	int fdistance;
    	int fNodeId;
    	int routing_tab[MAX_NUMBER_OF_NODES][3];
    };
        
    struct QMessage qmesg;
        
	qmesg.nodeId = 0;
	qmesg.fdistance = 0;

	for(int i =0 ; i<10 ; i++)
	{
		for(int j =0 ; j<3 ; j++)
		{
			qmesg.routing_tab[i][j] = 0;
		}
	}

    if (argc != 2)
    {
        printf("ERROR: Usage: talker hostname message\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0)
    {
        printf("ERROR: getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("TestNode: socket");
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        printf("ERROR: TestNode: failed to bind socket\n");
        return 2;
    }

    if ((numBytesSent = sendto(sockfd, sendbuf, MAXBUFLEN-1, 0, p->ai_addr, p->ai_addrlen)) == -1)
    {
        perror("TestNode: sendto");
        exit(1);
    }
    
    //printf("TestNode: sent %d bytes to %s\n", numbytes, argv[1]);
    
    if ((numBytesReceived = recvfrom(sockfd, &qmesg, MAXBUFLEN-1, 0, p->ai_addr, &(p->ai_addrlen))) == -1)
    {
            perror("TestNode: recvfrom");
            exit(1);
    }
    
   printf("\n\n\t-------------------------------------------------------\n");
   printf("\t\tCurrentNodeId: %d \n\t\tCurrentNodeDegree: %d \n\t\tFarthestNodeId: %d \n\t\tFarthest distance: %d \n",
		   qmesg.nodeId,
		   qmesg.nodeDegree,
		   qmesg.fNodeId,
		   qmesg.fdistance);

   printf("\t-------------------------------------------------------\n\n\n");
   printf("\t-------------------------------------------------------\n");
   printf("\t\t\tROUTING TABLE\n");
   printf("\t-------------------------------------------------------\n");
   printf("\tNODE ID\t\t\tDISTANCE\t\tNEXT HOP\n");

   for(int i =1 ; i<MAX_NUMBER_OF_NODES ; i++)
   {
	   if(qmesg.routing_tab[i][1] != 0xFFFF && qmesg.routing_tab[i][2] != 0xFFFF)
	   {
		   printf("\n");

		   for(int j=0 ; j<3 ; j++)
		   {
			   printf("\t%d\t\t",qmesg.routing_tab[i][j]);
		   }
	   }
   }

   printf("\n");
   printf("\t------------------------------------------------------\n\n\n");

   freeaddrinfo(servinfo);

   close(sockfd);

   return 0;
}
