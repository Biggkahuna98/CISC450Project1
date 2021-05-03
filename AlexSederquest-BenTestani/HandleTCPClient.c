#include <stdio.h> /* for printf() and fprintf() */
#include <stdlib.h>
#include <arpa/inet.h> /*for sockaddr_in and inet_addr()*/
#include <unistd.h> /* for close() */
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <time.h>
#include "TCPPacket.h"

#define RCVBUFSIZE 84 /* Size of receive buffer */

int tries = 0; // times a packet was sent for the timeout signal handler

void DieWithError(char *errorMessage);
int SimulateLoss(float packetLossRatio);
int power(int x, int y);
void CatchAlarm(int ignored); // handler for SIGALRM

int HandleTCPClient(int clntSocket, int servSocket, struct sockaddr_in servaddr, struct sockaddr_in cliaddr, int tout, float plr)
{
	// Seed random number generator with current time
	time_t t;
	srand((unsigned) time(&t));
	float pktlossratio = plr;
	char echoBuffer[RCVBUFSIZE];     /* Buffer for echo string */
	int recvMsgSize;
	struct sigaction myAction; // setting for signal handler

	// Create timeout struct
	struct timeval timeout;
	timeout.tv_sec = tout;
	timeout.tv_usec = power(10, tout);

	// create this temp packet for receiving the file name from the client
	tcp_packet filenamepacket;

	/* Receive message from client */
	// if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
	// 	DieWithError("recv() failed");
	recvMsgSize = recvfrom(clntSocket, echoBuffer, sizeof(echoBuffer), 0, (struct sockaddr_in*)NULL, NULL);

	// Convert the byte array (echoBuffer) back into a tcp_packet with the name of filenamepacket
	memcpy(&filenamepacket, echoBuffer, sizeof(tcp_packet));
	printf("Packet %d received with %d data bytes\n", filenamepacket.pack_seq_num, filenamepacket.count);
	printf("%s is the filename\n", filenamepacket.data);

	// Create the file pointer for reading in
	FILE* filePointer;
	int fileBufferLength = 84;
	char fileBuffer[fileBufferLength];
	char fileName[RCVBUFSIZE];

	// Open the file sent from the client
	// FIXME LATER
	//filePointer = fopen(filenamepacket.data, "r");
	filePointer = fopen("taco.txt", "r");

	// Packet struct
	tcp_packet pkt;
	memset(&pkt, 0, sizeof(tcp_packet));
	pkt.count = 7;
	pkt.pack_seq_num = 1;
	fflush(stdout);

	signal(SIGPIPE, SIG_IGN);

	// make buffer (byte stream)
	unsigned char *buff=(char*)malloc(sizeof(pkt));

	// Create the ACK packet
	ack_packet ack;
	memset(&ack, 0, sizeof(ack_packet));
	// make ack buffer (byte stream)
	unsigned char *ack_buff=(char*)malloc(sizeof(ack));

	// Set receive timeout
	setsockopt(clntSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	int test;
	//getsockopt(clntSocket, SOL_SOCKET, SO_RCVTIMEO, &test, sizeof(test));
	//printf("getsockopt: %d", test);

	myAction.sa_handler = CatchAlarm;
	if (sigfillset(&myAction.sa_mask) < 0) // block everything in handler
		DieWithError("sigfillset() failed");
	myAction.sa_flags = 0;

	if (sigaction(SIGALRM, &myAction, 0) < 0)
		DieWithError("sigaction() failed for SIGALRM");

	int total = 0;
	int len = 0;
	int respStringLen;
	// Read in the file line by line
	// Create a packet from each line
	// Send the packet
	while(fgets(fileBuffer, fileBufferLength, filePointer)) {
		// Copy the line into the packet's data
		strcpy(pkt.data, fileBuffer);
		// set the count to the length of the line (the byte count)
		pkt.count = strlen(fileBuffer);
		pkt.pack_seq_num = pkt.pack_seq_num == 1 ? 0 : 1;
		//pkt.count =  htons(strlen(fileBuffer));
		//pkt.pack_seq_num = htons(pkt.pack_seq_num);

		// Convert the packet into a buffer array (of bytes) named buff
		memcpy(buff, (const unsigned char*)&pkt, sizeof(pkt));
		printf("Packet %d generated for transmission with %d data bytes\n", pkt.pack_seq_num, pkt.count);
		if (SimulateLoss(pktlossratio) == 0) {
			// Send the buffer (buff)
			//send(clntSocket, buff, sizeof(pkt), 0);
			len = sizeof(cliaddr);
			//sendto(clntSocket, buff, sizeof(pkt), 0, (struct sockaddr_in*)&cliaddr, &len);
			sendto(clntSocket, buff, sizeof(pkt), 0, (struct sockaddr_in*)&cliaddr, len);
			printf("Packet %d successfully transmitted with %d data bytes\n", pkt.pack_seq_num, pkt.count);
			total += pkt.count;

			// Clear the buffer
			memset(buff, 0, sizeof(buff));
			printf("\n");
			fflush(stdout);

			// Receive ACK
			recvfrom(clntSocket, ack_buff, sizeof(ack_buff), 0, (struct sockarr_in*)NULL, NULL);
			if (errno == EINTR) {
				printf("hi, i timed out lol\n");
			}
			//test = getsockopt(clntSocket, SOL_SOCKET, SO_RCVTIMEO, NULL, NULL);
			//printf("getsockopt: %d\n", test);
			recvfrom(clntSocket, ack_buff, sizeof(ack_buff), 0, (struct sockarr_in*)NULL, NULL));
			if (errno == EAGAIN) {
					printf("TIMEOUT HITTTTT\n");
					printf("Packet %d generated for re-transmitted with %d data bytes\n", pkt.pack_seq_num, pkt.count);
					// now resend it
					sendto(clntSocket, buff, sizeof(pkt), 0, (struct sockaddr_in*)&cliaddr, len);
					total += pkt.count;
					printf("Packet %d successfully transmitted with %d data bytes\n", pkt.pack_seq_num, pkt.count);
			}
			// while ((respStringLen = recvfrom(clntSocket, ack_buff, sizeof(ack_buff), 0, (struct sockarr_in*)NULL, NULL)) < 0) {
			// 	if (errno == EAGAIN) {
			// 		printf("TIMEOUT HITTTTT\n");
			// 		printf("Packet %d generated for re-transmitted with %d data bytes\n", pkt.pack_seq_num, pkt.count);
			// 		// now resend it
			// 		sendto(clntSocket, buff, sizeof(pkt), 0, (struct sockaddr_in*)&cliaddr, len);
			// 		total += pkt.count;
			// 		printf("Packet %d successfully transmitted with %d data bytes\n", pkt.pack_seq_num, pkt.count);
			// 	}
			// }
			

			// alarm(tout);
			// while ((respStringLen = recvfrom(clntSocket, ack_buff, sizeof(ack_buff), 0, (struct sockarr_in*)NULL, NULL)) < 0) {
			// 	// alarm went off if true
			// 	if (errno == EINTR) {
			// 		// dont simulate loss for retransmission
			// 		printf("Packet %d generated for re-transmitted with %d data bytes\n", pkt.pack_seq_num, pkt.count);
			// 		// now resend it
			// 		sendto(clntSocket, buff, sizeof(pkt), 0, (struct sockaddr_in*)&cliaddr, len);
			// 		total += pkt.count;
			// 		printf("Packet %d successfully transmitted with %d data bytes\n", pkt.pack_seq_num, pkt.count);
			// 	} else {
			// 		//DieWithError("recvfrom() flopped");
			// 	}
			// }
			// // if the loop above doesnt go through, cancel the timeout
			// // in other words, recvfrom() got stuff
			// alarm(0);
		
			// Convert the byte array (echoBuffer) back into a tcp_packet with the name of pkt
			memcpy(&ack, ack_buff, sizeof(ack_packet));
			printf("ACK %d received\n", ack.ack_seq);
		} else {
			printf("Packet %d lost\n\n", pkt.pack_seq_num);
		}
		
	}
	// Set the data for the EOT packet
	strcpy(pkt.data, "");
	pkt.count = 0;
	//pkt.count =  htons(strlen(fileBuffer));
	//pkt.pack_seq_num = htons(pkt.pack_seq_num);
	// Convert the packet into a buffer array (of bytes) named buff
	memcpy(buff, (const unsigned char*)&pkt, sizeof(pkt));
	// Send the buffer
	sendto(clntSocket, buff, sizeof(pkt), 0, (struct sockaddr_in*)&cliaddr, len);
	printf("End of Transmission Packet with sequence number %d transmitted with %d data bytes\n", pkt.pack_seq_num, pkt.count);
	printf("Total number of data bytes transmitted: %d\n", total);

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

// Simulating the loss of a packet with the packet loss ration provided by the user
int SimulateLoss(float packetLossRatio) {
	// rand() % (upper - lower + 1) + lower for random number between 1 and 0
	if (rand() % (1 - 0 + 1) + 0 < packetLossRatio)
		return 1;
	else
		return 0;
}

// Power function, importing math libary didn't work out so just wrote it
int power(int x, int y) {
	int ret = 1;
	for (int i = 0; i < y; i++)
		ret *= x;
	return ret;
}

// Handler for SIGALRM
void CatchAlarm(int ignored) {
	printf("ACK NEVER RECEIVED, SIGALRM WENT OFF\n");
}