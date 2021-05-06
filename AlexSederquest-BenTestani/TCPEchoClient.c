


#include <stdio.h> /*for printf() and fprintf()*/
#include <sys/socket.h> /*for socket(), connect(), send(), and recv()*/
#include <arpa/inet.h> /*for sockaddr_in and inet_addr()*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "TCPPacket.h"

#define RCVBUFSIZE 84 /*Size of receive buffer*/

void DieWithError(char *errorMessage);
int SimulateACKLoss(float ackLossRatio);

int main(int argc, char *argv[])
{
	// Seed random number generator with current time
	time_t t;
	srand((unsigned) time(&t));
	
	float acklossratio = 0.0f;
	int sock; /*Socket descriptor*/
	struct sockaddr_in echoServAddr;
	unsigned short echoServPort;
	char *servIP;
	char echoString[80];
	char echoBuffer[RCVBUFSIZE];
	unsigned int echoStringLen;
	int bytesRcvd, totalBytesRcvd;

	if (argc != 5)
	{
			fprintf(stderr, "Usage: %s <Server IP> <Port> <File> <Ack Loss Ratio between 0 and 1>\n"), argv[0];
			exit(1);
	}

	// Get the file name to request from the server
	// printf("Enter file to send: "); // assume valid input
	// scanf("%s", echoString);

	servIP = argv[1];
	strcpy(echoString, argv[3]);
	acklossratio = strtof(argv[4], NULL);
	echoServPort = atoi(argv[2]);

	/*Create a socket using TCP*/
	if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
		DieWithError("socket() failed");

	fprintf("Filename: %s\n", echoString);
	fflush(stdout);

	/*Construct the server address structure*/
	memset(&echoServAddr, 0, sizeof(echoServAddr));
	echoServAddr.sin_family = AF_INET;
	echoServAddr.sin_addr.s_addr = inet_addr(servIP);
	echoServAddr.sin_port = htons(echoServPort);

	/*Establish connection to echo server*/
	if (connect(sock, (struct scokaddr*) &echoServAddr, sizeof (echoServAddr))<0)
		DieWithError("connect( ) failed");

	// Create the pkt
	// This packet is for receiving data from the server as well as
	// sending the original packet with the file name to the server
	tcp_packet pkt;
	memset(&pkt, 0, sizeof(tcp_packet));

	// Set up the data for the packet to transmit the file name to the server
	echoStringLen = strlen(echoString);
	pkt.count = echoStringLen;
	pkt.pack_seq_num = 0;
	strcpy(pkt.data, echoString);
	// make buffer (byte stream)
	unsigned char *buff=(char*)malloc(sizeof(pkt));
	// Copy the pkt into a byte array
	memcpy(buff, (const unsigned char*)&pkt, sizeof(pkt));
	/*send the packet with the file name*/
	sendto(sock, buff, sizeof(tcp_packet), 0, (struct sockaddr_in*)NULL, sizeof(echoServAddr));
	printf("Packet %d transmitted with %d data bytes\n\n", pkt.pack_seq_num, pkt.count);
	fflush(stdout);

	// Create the file pointer for writing to
	FILE* filePointer;

	// Open the file sent from the client
	filePointer = fopen("out.txt", "w+");

	// Create the ACK packet
	ack_packet ack;
	
	// make ack buffer (byte stream)
	unsigned char *ack_buff=(char*)malloc(sizeof(ack_packet));
	memset(ack_buff, 0, sizeof(ack_packet));

	// Loop for receiving data back from the server
	int totalinitial = 0;
	int total = 0;
	int num_pkts = 0;
	int num_dup_pkts = 0;
	int num_orig_pkts = 0;
	int num_success_ack = 0;
	int num_failed_ack = 0;
	int total_ack = 0;
	int lastSeqNum = 1;
	int lastPktCount = 1;
	while (1)
	{
		// Receive data from server, store in the buffer named echoBuffer
		// if((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE, 0))<=0)
		// 	DieWithError("recv() failed or connection closed prematurely");
		recvfrom(sock, buff, sizeof(tcp_packet), 0, (struct sockarr_in*)NULL, NULL);
		

		// Convert the byte array (echoBuffer) back into a tcp_packet with the name of pkt
		memcpy(&pkt, buff, sizeof(tcp_packet));
		if (lastSeqNum == pkt.pack_seq_num && lastPktCount == pkt.count) {
			printf("Duplicate packet %d received with %d data bytes\n", pkt.pack_seq_num, pkt.count);
			num_dup_pkts++;
		} else {
			printf("Packet %d received with %d data bytes\n", pkt.pack_seq_num, pkt.count);
			fflush(stdout);
			totalinitial += pkt.count;
			num_orig_pkts++;
		}
		lastSeqNum = pkt.pack_seq_num;
		lastPktCount = pkt.count;
		num_pkts++;
		total += pkt.count;
		
		// Make a nice space for readability in the terminal
    	//printf("\n");
		//fflush(stdout);

		// Make an output buffer to write to the file
		char outputBuffer[81];
		strcpy(outputBuffer, pkt.data);
		// Write the line to the file out.txt
		fprintf(filePointer, outputBuffer);
		printf("Packet %d delivered to user\n", pkt.pack_seq_num);
		fflush(stdout);

		// send ACK
		ack.ack_seq = pkt.pack_seq_num;
		// Copy the pkt into a byte array
		memcpy(ack_buff, (const unsigned char*)&ack, sizeof(ack_packet));
		printf("ACK %d generated for transmission\n", ack.ack_seq);
		fflush(stdout);
		if (SimulateACKLoss(acklossratio) == 0) {
			sendto(sock, ack_buff, sizeof(ack_packet), 0, (struct sockaddr_in*)NULL, sizeof(echoServAddr));
			printf("ACK %d successfully transmitted\n", ack.ack_seq);
			fflush(stdout);
			num_success_ack++;
		} else {
			printf("Ack %d lost\n", ack.ack_seq);
			fflush(stdout);
			num_failed_ack++;
		}
		total_ack++;
		printf("\n");
		fflush(stdout);
		//totalBytesRcvd += bytesRcvd;
				// Check for the EOT packet
		if(pkt.count == 0){
			printf("End of Transmission Packet with sequence number %d received with %d data bytes\n", pkt.pack_seq_num, pkt.count);
			fflush(stdout);
			break;
		}
	}
	
	printf("Total number of data packets received successfully: %d\n", num_pkts);
	printf("Number of duplicate data packets received: %d\n", num_dup_pkts);
	printf("Number of data packets received successfully, not including duplicates: %d\n", num_orig_pkts);
	printf("Number of ACKs transmitted without loss: %d\n", num_success_ack);
	printf("Number of ACKs generated but dropped due to loss: %d\n", num_failed_ack);
	printf("Total number of ACKs generated (with and without loss): %d\n", total_ack);
	// Total bytes received as well as closing the socket
	printf("Total number of data bytes received: %d\n", total);
	// Close the output file
	fclose(filePointer);
	free(buff);
	free(ack_buff);
	// Close the socket
	close(sock);
	exit(0);
}

int SimulateACKLoss(float ackLossRatio) {
	// rand() % (upper - lower + 1) + lower for random number between 1 and 0
	if (rand() % 2 < ackLossRatio)
		return 1;
	else
		return 0;
}