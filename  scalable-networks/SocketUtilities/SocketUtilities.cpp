#include <SocketUtilities.h>

/****************************************************************************************
 /** sendDataOnTCP: Function to send data completely
  *
  * @param[in] socketFd This is the file descriptor to which data has to be written
  *
  * @param[in] *sendBuffer Pointer to the message
  *
  * @param[in/out] length This is the length of the data to be sent, which is also returned back
  *
  * @return 0 if PASSED or -1 if FAILED
 *
 ****************************************************************************************/
int sendDataOnTCP(int socketFd, char *sendBuffer, int *length)
{
	int total = 0; // how many bytes we've sent
	int bytesleft = *length; // how many we have left to send
	int n;

	while(total < *length)
	{
		n = send(socketFd, sendBuffer+total, bytesleft, 0);

		if (n == -1)
		{
			printf("ERROR: sendAll, send failed, ECODE %s\n", strerror(errno));
			break;
		}

		total += n;
		bytesleft -= n;
	}

	*length = total; /* Return number actually sent here */

	return (n==-1)?-1:0; /* return -1 on failure, 0 on success */
}

/****************************************************************************************
 /** receiveDataOnTCP: Function to send data completely
  *
  * @param[in] socketFd This is the file descriptor to which data has to be written
  *
  * @param[in] *receiveBuffer Pointer to the receiver buffer
  *
  * @param[in/out] length This is the length of the data to be sent, which is also returned back
  *
  * @return 0 if PASSED or -1 if FAILED
 *
 ****************************************************************************************/
int receiveDataOnTCP(int socketFd, char *receiveBuffer, int *length)
{
	int total = 0; 				// How many bytes we've received
	int bytesleft = *length; 	// How many we have left to receive
	int n;

	while(total < *length)
	{
		n = recv(socketFd, receiveBuffer+total, bytesleft, 0);

		if (n == -1)
		{
			printf("ERROR: receiveAll, recv failed, ECODE %s\n", strerror(errno));
			break;
		}

		total += n;
		bytesleft -= n;
	}

	*length = total; /* Return number actually received here */

	return (n==-1)?-1:0; /* return -1 on failure, 0 on success */
}

