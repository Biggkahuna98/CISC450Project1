#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> /*for socket(), connect(), send(), and recv()*/
#include <arpa/inet.h>
#include "TCPPacket.h"

int main() {
    tcp_packet pkt = {5,2,{'a','a','a','a','a','\n'}};

    unsigned char *buff=(char*)malloc(sizeof(pkt));
    
    memcpy(buff, (const unsigned char*)&pkt, sizeof(pkt));
    printf("size of pkt: %u\n", sizeof(pkt));
    printf("size of tcp_packet: %u\n", sizeof(tcp_packet));

    printf("Byte array is as follows\n");
    for (int i = 0; i < sizeof(pkt); i++) {
        printf("%02X ", buff[i]);
    }
    printf("\n");

    tcp_packet pkt2;
    memcpy(&pkt2, buff, sizeof(tcp_packet));
    printf("Count: %d\n", pkt2.count);
    printf("Pack_seq_num: %d\n", pkt2.pack_seq_num);

    free(buff);

    short x = 100;
    printf("x: %d\n", x);
    x = htons(x);
    printf("htons(x): %d\n", x);
    x = ntohs(x);
    printf("ntohs(x): %d\n", x);

    return 0;
}