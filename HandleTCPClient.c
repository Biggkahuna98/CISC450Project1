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

	tcp_packet filenamepacket;

	/* Receive message from client */
	if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
		DieWithError("recv() failed");

	memcpy(&filenamepacket, echoBuffer, sizeof(tcp_packet));
	// print file name
	printf("%s\n", filenamepacket.data);
	fflush(stdout);
	// Print the filename
	//printf("%d\n", strlen(echoBuffer));
	// Our janky way of fixing the buffer
	//echoBuffer[strlen(echoBuffer) - 2] = '\0';
	//printf("%s\n", echoBuffer);
	//fflush(stdout);

	// Read in the file into byte array and then create packets with em
	FILE* filePointer;
	int fileBufferLength = 84;
	char fileBuffer[fileBufferLength];

	// FIX ME LATER
	char fileName[RCVBUFSIZE];
	// sprintf(fileName, echoBuffer);
	filePointer = fopen(filenamepacket.data, "r");
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
		//pkt.count =  strlen(fileBuffer);
		//pkt.pack_seq_num = pkt.pack_seq_num;
		printf("Filebuffer size: %d\n", strlen(fileBuffer));
		printf("pkt_seq_num: %d\n", pkt.pack_seq_num);
		fflush(stdout);
		pkt.count =  strlen(fileBuffer);
		//pkt.pack_seq_num = htons(pkt.pack_seq_num);

		printf("count: %d\n", pkt.count);
		printf("pkt_seq_num: %d\n", pkt.pack_seq_num);
		fflush(stdout);

		// Copy the pkt into a byte array
		memcpy(buff, (const unsigned char*)&pkt, sizeof(pkt));
		// Send the byte array over the socket
		send(clntSocket, buff, sizeof(pkt), 0);

		// Clear the byte array
		memset(buff, 0, sizeof(buff));
		pkt.pack_seq_num++;
		printf("\n\n");
		fflush(stdout);
	}

	// Close and free stuff
	free(buff);
	fclose(filePointer);
	close(clntSocket); /* Close client socket */
}