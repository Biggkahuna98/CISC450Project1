


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

int main(int argc, char *argv[])
{
	// Seed random number generator with current time
	srand(time(0));
	float acklossratio = 0.15f;
	int sock; /*Socket descriptor*/
	struct sockaddr_in echoServAddr;
	unsigned short echoServPort;
	char *servIP;
	char echoString[RCVBUFSIZE];
	char echoBuffer[RCVBUFSIZE];
	unsigned int echoStringLen;
	int bytesRcvd, totalBytesRcvd;

	if ((argc<3) || (argc>3))
	{
			fprintf(stderr, "Usage: %s <Server IP> <Echo Port>\n"), argv[0];
			exit(1);
	}

	// Get the file name to request from the server
	printf("Enter file to send: "); // assume valid input
	scanf("%s", echoString);

	servIP = argv[1];

	if(argc == 3)
		echoServPort = atoi(argv[2]);
	else
		echoServPort = 7;

	/*Create a socket using TCP*/
	if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
		DieWithError("socket() failed");

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
	//send(sock, buff, sizeof(pkt), 0);
	sendto(sock, buff, sizeof(buff), 0, (struct sockaddr_in*)NULL, sizeof(echoServAddr));
	printf("Packet %d transmitted with %d data bytes\n", pkt.pack_seq_num, pkt.count);

	// Create the file pointer for writing to
	FILE* filePointer;

	// Open the file sent from the client
	filePointer = fopen("out.txt", "w+");

	// Create the ACK packet
	ack_packet ack;
	memset(&ack, 0, sizeof(ack_packet));
	// make ack buffer (byte stream)
	unsigned char *ack_buff=(char*)malloc(sizeof(ack));

	// Loop for receiving data back from the server
	int total = 0;
	printf("Received: \n");
	while (1)
	{
		// Receive data from server, store in the buffer named echoBuffer
		// if((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE, 0))<=0)
		// 	DieWithError("recv() failed or connection closed prematurely");
		bytesRcvd = recvfrom(sock, echoBuffer, sizeof(echoBuffer), 0, (struct sockarr_in*)NULL, NULL);
		if (SimulateLoss(acklossratio) == 0) {
			// send ACK
			if (ack.ack_seq == 0)
				ack.ack_seq = 1;
			else
				ack.ack_seq = 0;
			// Copy the pkt into a byte array
			memcpy(ack_buff, (const unsigned char*)&ack, sizeof(ack));
			sendto(sock, ack_buff, sizeof(ack_buff), 0, (struct sockaddr_in*)NULL, sizeof(echoServAddr));
		} else {
			printf("Ack lost, rip\n");
		}
		totalBytesRcvd += bytesRcvd;
		
		// Convert the byte array (echoBuffer) back into a tcp_packet with the name of pkt
		memcpy(&pkt, echoBuffer, sizeof(tcp_packet));

		// Check for the EOT packet
		if(pkt.count == 0){
			printf("End of Transmission Packet with sequence number %d received with %d data bytes\n", pkt.pack_seq_num,pkt.count);
			break;
		}
		printf("Packet %d received with %d data bytes\n", pkt.pack_seq_num, pkt.count);
		total += pkt.count;
		
		// Make a nice space for readability in the terminal
    	printf("\n");
		fflush(stdout);

		// Make an output buffer to write to the file
		char outputBuffer[81];
		strcpy(outputBuffer, pkt.data);
		// Write the line to the file out.txt
		fprintf(filePointer, outputBuffer);
	}

	// Total bytes received as well as closing the socket
	printf("Total number of data bytes received: %d\n", total);
	printf("\n");
	fflush(stdout);
	// Close the output file
	fclose(filePointer);
	// Close the socket
	close(sock);
	exit(0);
}

int SimulateACKLoss(float ackLossRatio) {
	return 0;
	// rand() % (upper - lower + 1) + lower for random number between 1 and 0
	if (rand() % (1 - 0 + 1) + 0 < ackLossRatio)
		return 1;
	else
		return 0;
}