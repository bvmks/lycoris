#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "ms_communicate.h"
#include "crc32.h"

int ms_send(int sockfd, struct ipv4_address *dst, struct ms_packet* packet)
{
    char *buf, *cur;
    int buf_len, hash, sent;

    struct sockaddr_in addr;
    socklen_t addr_len;

    addr.sin_family = AF_INET;
    addr.sin_port = dst->port;
    addr.sin_addr.s_addr = dst->address;
    addr_len = sizeof(addr);
    
    buf_len = sizeof(struct ms_header) + packet->data_size + 4;
    buf = malloc(buf_len);
    
    *((struct ms_header*) buf)->type = packet->header.type;

    hash = crc32(data, buf_len); /* INSERT HASH FUNC HERE */

    *cur = hash;
    *(cur+4) = '\0';

    sent = sendto(sockfd, data, buf_len, 0, (struct sockaddr*)&addr, addr_len);
    free(buf);
    return sent;
}
