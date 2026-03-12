#ifndef _MS_COMMUNICATE_H
#define _MS_COMMUNICATE_H

#include <sys/socket.h>


#include "ms_packets.h"
#include "liblyc.h"

enum {
    ms_packet_size = 508,
    ms_port        = 24880
};

/*
*   packs the msg and ms_msg_header and then
*   sends it via UDP to destination, provided by addr
*   returns amount of sent bytes, -1 if error occured
*/
int ms_send(int sockfd, struct ipv4_address* dst, unsigned short msg_type, char* msg, int msg_len);

/*
*   receives the msg and stores it in 
*   pointer to struct ms_packet
*   returns 0 if everything ok, otherwise returns -1
*/
int ms_recv(int sockfd, struct ms_packet* in_packet);

#endif
