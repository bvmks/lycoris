#ifndef _MS_PACKETS_H
#define _MS_PACKETS_H

#include "ms_headers.h"
#include <sys/socket.h>

/*
*   represents info about received packet from ms_recv()
*   contains header, source address, and then data
*/
struct ms_packet {
    struct ms_msg_header header;
    struct ipv4_address addr;
    char* data;
    int data_size;
};

#endif // !_MS_PACKETS_H
