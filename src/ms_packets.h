#ifndef _MS_PACKETS_H
#define _MS_PACKETS_H

#include <netinet/in.h>
#include <sys/socket.h>

#include "ms_headers.h"
#include "addrport.h"

struct ms_packet {
    struct ms_header header;
    char* data;
    int data_size;
};

/*
*   represents info about received packet from ms_recv()
*   contains header, source address, and then data
*/
struct ms_received_packet {
    struct addrport src_addr;
    struct ms_packet packet;
};

void packet_init(struct ms_packet*);

void packet_free(struct ms_packet*);

#endif
