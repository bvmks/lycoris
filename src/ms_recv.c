#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "ms_communicate.h"

int ms_recv(int sockfd, struct ms_packet *in_packet)
{
    int received;
    char* buf = malloc(ms_packet_max_size);
    char* buf_end = buf + received;
    struct sockaddr_in src;
    struct ipv4_address tmp;
    socklen_t src_len = sizeof(src);
    received = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&src, &src_len);
    
    if (received == 0) {
        in_packet->addr.address = src.sin_addr.s_addr;
        in_packet->addr.port    = src.sin_port;
        in_packet->data_size    = 0;
        return 0;
    }

    if (received == -1) {
        return -1;
    }

    int src_hash = ms_crc32_check(buf, received - 4), tmp_hash = *(buf + received - 4);
    if (src_hash != tmp_hash) {
        /* REQUEST PACKET AGAIN */
    }
    
    tmp.address = src.sin_addr.s_addr;
    tmp.port = src.sin_port;
    in_packet->addr = tmp;
    in_packet->data = buf + ms_basic_header_len;
    in_packet->data_size = received - ms_basic_header_len;
    ms_unpack_header(buf, &(in_packet->header));

    return received;
}
