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
void timeout_in_seconds(int t); // non-socket timeout

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
	recvMsgSize = recvfrom(clntSocket, echoBuffer, sizeof(echoBuffer), 0, (struct sockaddr_in*)NULL, NULL);

	// Convert the byte array (echoBuffer) back into a tcp_packet with the name of filenamepacket
	memcpy(&filenamepacket, echoBuffer, sizeof(tcp_packet));
	printf("Packet %d received with %d data bytes\n\n", filenamepacket.pack_seq_num, filenamepacket.count);
	//printf("%s is the filename\n\n", filenamepacket.data);

	// Create the file pointer for reading in
	FILE* filePointer;
	int fileBufferLength = 80;
	char fileBuffer[80];

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

	// Create the ACK packet
	ack_packet ack;
	memset(&ack, 0, sizeof(ack_packet));
	// make ack buffer (byte stream)
	unsigned char *ack_buff=(char*)malloc(sizeof(ack));
	memset(ack_buff, 0, sizeof(ack));

	// Setup signal handler for the timeout
	myAction.sa_handler = CatchAlarm;
	if (sigfillset(&myAction.sa_mask) < 0) // block everything in handler
		DieWithError("sigfillset() failed");
	myAction.sa_flags = 0;

	if (sigaction(SIGALRM, &myAction, 0) < 0)
		DieWithError("sigaction() failed for SIGALRM");

	int total = 0;
	int total_initial_tr = 0;
	int total_initial_bytes = 0;
	int num_pkt_retr = 0;
	int num_pkt_lost = 0;
	int num_ack_recv = 0;
	int num_timeouts = 0;

	int len = sizeof(cliaddr);
	int respStringLen = 0;
	int retransmit = -1;
	// 0 = generate new packet
	// 1 = waiting on ack
	// 2 = ack received
	// 3 = send a packet
	// 4 = do a timeout
	// 5 = END OF LOOP
	int state = 0;
	// Read in the file line by line
	// Create a packet from each line
	// Send the packet
	while(state != 5) {
		switch(state) {
			case 0: // generate new packet
				if (fgets(fileBuffer, fileBufferLength, filePointer)  == NULL) {
					state = 5;
					break; // exit point for loop
				}
			
				// Copy the line into the packet's data
				strcpy(pkt.data, fileBuffer);
				// set the count to the length of the line (the byte count)
				pkt.count = strlen(fileBuffer);
				pkt.pack_seq_num = pkt.pack_seq_num == 1 ? 0 : 1;
				//pkt.count =  htons(strlen(fileBuffer));
				//pkt.pack_seq_num = htons(pkt.pack_seq_num);

				// Convert the packet into a buffer array (of bytes) named buff
				memcpy(buff, (const unsigned char*)&pkt, sizeof(pkt));
				total_initial_tr++;
				total_initial_bytes += pkt.count;
				printf("Packet %d generated for transmission with %d data bytes\n", pkt.pack_seq_num, pkt.count);
				state = 3;
				break;

			case 1: // waiting on ack
				alarm(tout);
				while ((respStringLen = recvfrom(clntSocket, ack_buff, sizeof(ack_packet), 0, (struct sockarr_in*)NULL, NULL)) < 0) {
					// alarm went off if true
					if (errno == EINTR) {
						printf("Timeout expired for packet numbered %d\n", pkt.count);
						retransmit = 1;
						num_pkt_retr++;
						num_timeouts++;
						state = 3; // try to resend packet
						break;	
					}
				}
				if (state == 3)
					break;
				// Turn alarm off, we received an ACK
				alarm(0);
				state = 2;
				break;
			case 2: // received ack
				// Convert the byte array (echoBuffer) back into a tcp_packet with the name of pkt
				memcpy(&ack, ack_buff, sizeof(ack_packet));
				printf("ACK %d received\n\n", ack.ack_seq);
				// Clear the buffer
				memset(buff, 0, sizeof(buff));
				num_ack_recv++;
				state = 0; // create a new packet to send from the next line in file
				break;
			case 3: // send a packet
				if (SimulateLoss(pktlossratio) == 0) {
					if (retransmit == 1) {
						printf("Packet %d generated for re-transmitted with %d data bytes\n", pkt.pack_seq_num, pkt.count);
						fflush(stdout);
						retransmit = 0;
					}
					sendto(clntSocket, buff, sizeof(pkt), 0, (struct sockaddr_in*)&cliaddr, len);
					total += pkt.count;
					printf("Packet %d successfully transmitted with %d data bytes\n", pkt.pack_seq_num, pkt.count);
					fflush(stdout);
					state = 1; // wait for ack
				} else {
					printf("Packet %d lost\n", pkt.pack_seq_num);
					retransmit = 1;
					fflush(stdout);
					num_pkt_lost++;
					state = 4; // timeout
				}
				break;
			case 4: // do a timeout then send again
				timeout_in_seconds(tout); // do a normal timeout, not a socket one
				state = 3;
				break;
			case 5: // break loop, end of file
				break;
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
	printf("Number of data packets generated for transmission (initial transmission only): %d\n", total_initial_tr);
	printf("Total number of data bytes generated for transmission, initial transmission only: %d\n", total_initial_bytes);
	printf("Total number of data packets generated for retransmission (initial transmissions plus retransmissions): %d\n", num_pkt_retr);
	printf("Number of data packets dropped due to loss: %d\n", num_pkt_lost);
	printf("Number of data packets transmitted successfully (initial transmissions plus retransmissions): %d\n", total_initial_tr+num_pkt_retr);
	printf("Number of ACKs received: %d\n", num_ack_recv);
	printf("Count of how many times timeout expired: %d\n", num_timeouts);
	printf("(NOT NEEDED?) Total number of data bytes transmitted: %d\n", total);

	fflush(stdout);

	// Clear the buffer
	memset(buff, 0, sizeof(buff));
	pkt.pack_seq_num++;

	// Free the buffer
	free(buff);
	free(ack_buff);
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
	//printf("ACK NEVER RECEIVED, SIGALRM WENT OFF\n");
}

void timeout_in_seconds(int t) {
	time_t start = time(0);
	for (;;) {
		if (time(0) > start + 5) {
			break;
		}
	}
}