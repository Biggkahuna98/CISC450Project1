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
	filePointer = fopen(fileBuffer, "r");
	// Packet struct
	tcp_packet pkt;
	memset(&pkt, 0, sizeof(tcp_packet));
	pkt.count = 7;
	pkt.pack_seq_num = 0;
	fflush(stdout);

	signal(SIGPIPE, SIG_IGN);

	// make buffer (byte stream)
	unsigned char *buff=(char*)malloc(sizeof(pkt));

	while(fgets(fileBuffer, fileBufferLength, filePointer)) {
		// Put the line of the file into the packet data section
		strcpy(pkt.data, fileBuffer);

		// Convert the data to the sending format
		pkt.count =  strlen(fileBuffer);
		pkt.pack_seq_num = pkt.pack_seq_num;
		pkt.count =  htons(strlen(fileBuffer));
		pkt.pack_seq_num = htons(pkt.pack_seq_num);

		// Copy the pkt into a byte array
		memcpy(buff, (const unsigned char*)&pkt, sizeof(pkt));
		// Send the byte array over the socket
		send(clntSocket, buff, sizeof(pkt), 0);

		// Clear the byte array
		memset(buff, 0, sizeof(buff));
		pkt.pack_seq_num++;
	}

	// Close and free stuff
	free(buff);
	fclose(filePointer);
	close(clntSocket); /* Close client socket */
}