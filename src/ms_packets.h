#ifndef _MS_PACKETS_H
#define _MS_PACKETS_H

#include "ms_headers.h"
#include <netinet/in.h>
#include <sys/socket.h>

/*
*   represents info about received packet from ms_recv()
*   contains header, source address, and then data
*/
struct ms_packet {
    struct ms_header header;
    struct ipv4_address src_addr;
    char* data;
    int data_size;
};

void init_packet(struct ms_packet*);

#endif
