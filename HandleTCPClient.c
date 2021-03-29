#include <stdio.h> /* for printf() and fprintf() */
#include <stdlib.h>
#include <unistd.h> /* for close() */
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
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

	// FIX ME LATER
	filePointer = fopen("taco.txt", "r");
	// Packet struct
	tcp_packet pkt;
	memset(&pkt, 0, sizeof(tcp_packet));
	pkt.count = 7;
	pkt.pack_seq_num = 0;
	fflush(stdout);

	signal(SIGPIPE, SIG_IGN);

	// make buffer (byte stream)
	unsigned char *buff=(char*)malloc(sizeof(pkt));

	int rcvmsgsizeold = recvMsgSize;
	while(fgets(fileBuffer, fileBufferLength, filePointer)) {
		printf("%s", fileBuffer);
		fflush(stdout);
		// Put the line of the file into the packet data section
		
		recvMsgSize = rcvmsgsizeold;
		strcpy(pkt.data, fileBuffer);
		memcpy(buff, (const unsigned char*)&pkt, sizeof(pkt));
		
		send(clntSocket, buff, sizeof(pkt), 0);

		//if (send(clntSocket, buff, sizeof(buff), MSG_NOSIGNAL) != recvMsgSize)
		//		DieWithError("send() failed");
		// while (recvMsgSize > 0) {
			
		// 	if ((recvMsgSize = recv(clntSocket, fileBuffer, RCVBUFSIZE, 0)) < 0)
		// 		DieWithError("recv() failed");
		// }
		memset(buff, 0, sizeof(buff));
		pkt.pack_seq_num++;
		//send(clntSocket, &pkt, sizeof(pkt), 0);
		// while (recvMsgSize > 0) {
		// 	if (send(clntSocket, &pkt, sizeof(tcp_packet), 0) != recvMsgSize)
		// 		DieWithError("send() failed");
		// 	if ((recvMsgSize = recv(clntSocket, fileBuffer, RCVBUFSIZE, 0)) < 0)
		// 		DieWithError("recv() failed");
		// }
	}

	free(buff);
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