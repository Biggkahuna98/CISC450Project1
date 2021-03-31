typedef struct TCP_PACKET {
    short count;
    short pack_seq_num;
    char data[80];
} tcp_packet;