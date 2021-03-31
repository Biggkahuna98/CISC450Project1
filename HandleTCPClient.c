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

	int rcvmsgsizeold = recvMsgSize;
	while(fgets(fileBuffer, fileBufferLength, filePointer)) {
		printf("%s", fileBuffer);
		fflush(stdout);
		// Put the line of the file into the packet data section
		
		recvMsgSize = rcvmsgsizeold;
		strcpy(pkt.data, fileBuffer);
		pkt.count =  strlen(fileBuffer);
		pkt.pack_seq_num = pkt.pack_seq_num;
		//pkt.count =  htons(strlen(fileBuffer));
		//pkt.pack_seq_num = htons(pkt.pack_seq_num);

		//if(pkt.count != 80){
			//pkt.data[pkt.count+1] = '\0';
		//}
		memcpy(buff, (const unsigned char*)&pkt, sizeof(pkt));
		
		send(clntSocket, buff, sizeof(pkt), 0);
		printf("Packet %d transmitted with %d data bytes\n", pkt.pack_seq_num, pkt.count);
		//if (send(clntSocket, buff, sizeof(buff), MSG_NOSIGNAL) != recvMsgSize)
		//		DieWithError("send() failed");
		// while (recvMsgSize > 0) {
			
		// 	if ((recvMsgSize = recv(clntSocket, fileBuffer, RCVBUFSIZE, 0)) < 0)
		// 		DieWithError("recv() failed");
		// }
		memset(buff, 0, sizeof(buff));
		pkt.pack_seq_num++;
		printf("\n\n");
		fflush(stdout);
	}
	strcpy(pkt.data, "\0");
	pkt.count =  -1;
	pkt.pack_seq_num = pkt.pack_seq_num;
	//pkt.count =  htons(strlen(fileBuffer));
	//pkt.pack_seq_num = htons(pkt.pack_seq_num);
	memcpy(buff, (const unsigned char*)&pkt, sizeof(pkt));
	send(clntSocket, buff, sizeof(pkt), 0);
	printf("End of Transmission Packet with sequence number %d transmitted with 1 data bytes\n", pkt.pack_seq_num);

	memset(buff, 0, sizeof(buff));
	pkt.pack_seq_num++;

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
	close(servSocket);
	return 0;
}