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

int HandleTCPClient(int clntSocket, int servSocket)
{
	char echoBuffer[RCVBUFSIZE];     /* Buffer for echo string */
	int recvMsgSize;

	// create this temp packet for receiving the file name from the client
	tcp_packet filenamepacket;

	/* Receive message from client */
	if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
		DieWithError("recv() failed");

	// Convert the byte array (echoBuffer) back into a tcp_packet with the name of filenamepacket
	memcpy(&filenamepacket, echoBuffer, sizeof(tcp_packet));
	printf("Packet %d received with %d data bytes\n", filenamepacket.pack_seq_num, filenamepacket.count);

	// Create the file pointer for reading in
	FILE* filePointer;
	int fileBufferLength = 84;
	char fileBuffer[fileBufferLength];
	char fileName[RCVBUFSIZE];

	// Open the file sent from the client
	filePointer = fopen(filenamepacket.data, "r");

	// Packet struct
	tcp_packet pkt;
	memset(&pkt, 0, sizeof(tcp_packet));
	pkt.count = 7;
	pkt.pack_seq_num = 1;
	fflush(stdout);

	signal(SIGPIPE, SIG_IGN);

	// make buffer (byte stream)
	unsigned char *buff=(char*)malloc(sizeof(pkt));

	int total = 0;
	// Read in the file line by line
	// Create a packet from each line
	// Send the packet
	while(fgets(fileBuffer, fileBufferLength, filePointer)) {
		// Copy the line into the packet's data
		strcpy(pkt.data, fileBuffer);
		// set the count to the length of the line (the byte count)
		pkt.count = strlen(fileBuffer);
		//pkt.count =  htons(strlen(fileBuffer));
		//pkt.pack_seq_num = htons(pkt.pack_seq_num);

		// Convert the packet into a buffer array (of bytes) named buff
		memcpy(buff, (const unsigned char*)&pkt, sizeof(pkt));
		
		// Send the buffer (buff)
		send(clntSocket, buff, sizeof(pkt), 0);
		printf("Packet %d transmitted with %d data bytes\n", pkt.pack_seq_num, pkt.count);
		
		total += pkt.count;

		// Clear the buffer
		memset(buff, 0, sizeof(buff));
		// Increment the sequence
		pkt.pack_seq_num++;
		printf("\n");
		fflush(stdout);
	}
	// Set the data for the EOT packet
	strcpy(pkt.data, "");
	pkt.count =  0;
	//pkt.count =  htons(strlen(fileBuffer));
	//pkt.pack_seq_num = htons(pkt.pack_seq_num);
	// Convert the packet into a buffer array (of bytes) named buff
	memcpy(buff, (const unsigned char*)&pkt, sizeof(pkt));
	// Send the buffer
	send(clntSocket, buff, sizeof(pkt), 0);
	printf("End of Transmission Packet with sequence number %d transmitted with 1 data bytes\n", pkt.pack_seq_num);
	printf("Total number of data bytes received: %d\n", total);

	// Clear the buffer
	memset(buff, 0, sizeof(buff));
	pkt.pack_seq_num++;

	// Free the buffer
	free(buff);
	// Close the file
	fclose(filePointer);

	// Close the sockets
	close(clntSocket);
	close(servSocket);
	return 0;
}