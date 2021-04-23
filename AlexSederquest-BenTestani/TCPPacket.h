typedef struct TCP_PACKET {
    short count;
    short pack_seq_num;
    char data[80];
} tcp_packet;

typedef struct ACK_PACKET {
    short ack_seq;
} ack_packet;