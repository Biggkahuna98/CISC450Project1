To build the project, simply type "make" into your terminal. This will build the binaries.
Then, to run the project, open 2 terminal windows.
In the first window run the server by typing "./TCPEchoServer <port>" where <port> is your desired port to use.
In the 2nd window, run the client by typing "./TCPEchoClient <server_ip> <port>" where <server_ip> is the ip of the server and <port> is the port of the server.
When you run the client it will prompt you to enter a file name. This file must be in the same directory the client and server are located in. Be sure to include the file extension, i.e. "taco.txt" is our example file.
Now the client will request the file from the server. When the client receives all of the file it will be saved as "out.txt".

The files included are:
README.txt
makefile
DieWithError.c
HandleTCPClient.c
TCPEchoClient.c
TCPEchoServer.c
TCPPacket.h
test.c
taco.txt