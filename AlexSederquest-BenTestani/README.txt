To build the project, simply type "make" into your terminal. This will build the binaries.
Then, to run the project, open 2 terminal windows.
In the first window run the server by typing "./TCPEchoServer <Port> <Timeout value in seconds> <Packet Loss Ratio between 0 and 1>" where <Port> is your desired port to use, <Timeout value in seconds> is the waiting for an ACK timeout, and <Packet Loss Ration between 0 and 1> is for simulating packet loss
In the 2nd window, run the client by typing "./TCPEchoClient <Server IP> <Port> <File> <Ack Loss Ratio>" where <Server IP> is the ip of the server and <Port> is the port of the server, <File> is the name of the file to receive from the server, <Ack Loss Ratio> is between 0 and 1 for simulating ACK loss by the client
The file you want to receive must be in the same directory the server is located in. Be sure to include the file extension, i.e. "taco.txt" is our example file.
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