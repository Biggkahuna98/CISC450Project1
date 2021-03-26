#include <stdio.h> /* for printf() and fprintf() */
#include <stdlib.h>
#include <unistd.h> /* for close() */
#include "TCPPacket.h"

#define RCVBUFSIZE 84 /* Size of receive buffer */

void DieWithError(char *errorMessage);

void HandleTCPClient(int clntSocket)
{
	char echoBuffer[RCVBUFSIZE];     /* Buffer for echo string */
	int recvMsgSize;

	/* Receive message from client */
	if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
		DieWithError("recv() failed");

	// Print the filename
	printf("%s\n", echoBuffer);
	fflush(stdout);

	// Read in the file into byte array and then create packets with em
	FILE* filePointer;
	int fileBufferLength = 84;
	char fileBuffer[fileBufferLength];

	filePointer = fopen(echoBuffer, "r");

	char sendByteArray[84];
	sendByteArray[0] = 
	while(fgets(fileBuffer, fileBufferLength, filePointer)) {
		printf("%s", fileBuffer);

		recvMsgSize = strlen(fileBuffer) + 1;
		while (recvMsgSize > 0) {
			if (send(clntSocket, fileBuffer, recvMsgSize, 0) != recvMsgSize)
				DieWithError("send() failed");
			if ((recvMsgSize = recv(clntSocket, fileBuffer, RCVBUFSIZE, 0)) < 0)
				DieWithError("recv() failed");
		}
	}

	fclose(filePointer);

	/* Send received string and receive again until end of transmission */
	// while (recvMsgSize > 0) /* zero indicates end of transmission */
	// {
	// 	/* Echo message back to client */
	// 	if (send(clntSocket, echoBuffer, recvMsgSize, 0) != recvMsgSize)
	// 		DieWithError("send() failed");

	// 	/* See if there is more data to receive */
	// 	if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
	// 		DieWithError("recv() failed");

	// }

	close(clntSocket); /* Close client socket */
}