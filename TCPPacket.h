typedef struct TCP_PACKET {
    unsigned short count;
    unsigned short pack_seq_num;
    char data[80];
} tcp_packet;