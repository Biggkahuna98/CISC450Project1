typedef struct tcp_packet {
    unsigned short count;
    unsigned short pack_seq_num;
    char data[80];
};