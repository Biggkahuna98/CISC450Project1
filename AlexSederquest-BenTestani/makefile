CC=gcc
CFLAGS=-Wall

all: TCPEchoClient TCPEchoServer

OTHER_OBSS = DieWithError.o HandleTCPClient.o

TCPEchoClient :	TCPEchoClient.o	$(OTHER_OBSS)

TCPEchoServer :	TCPEchoServer.o	$(OTHER_OBSS)

DieWithError.o : DieWithError.c
				$(CC) -c -g DieWithError.c

HandleTCPClient.o : HandleTCPClient.c
				$(CC) -c -g HandleTCPClient.c

TCPEchoClient.o: TCPEchoClient.c
				$(CC) -c -g TCPEchoClient.c

TCPEchoSever.o: TCPEchoSever.c 
				$(CC) -c -g TCPEchoSever.c

clean:
		rm -f	*.o TCPEchoClient TCPEchoServer test
				