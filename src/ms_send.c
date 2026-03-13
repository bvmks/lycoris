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
    int buf_len, sent;
    unsigned int hash;
    struct ms_header* fill_header;

    struct sockaddr_in addr;
    socklen_t addr_len;

    addr.sin_family = AF_INET;
    addr.sin_port = dst->port;
    addr.sin_addr.s_addr = dst->address;
    addr_len = sizeof(addr);
    
    buf_len = sizeof(struct ms_header) + packet->data_size + 4;
    buf = malloc(buf_len);
    fill_header = (struct ms_header*) buf;
    fill_header->type = packet->header.type;
    fill_header->s_id = htons(packet->header.s_id);
    fill_header->seq = htons(packet->header.seq);
    
    if(packet->data)
        memcpy(buf + sizeof(struct ms_header), packet->data, packet->data_size);
    
    *((unsigned int*)(buf + buf_len -4)) = 0;
    hash = crc32(buf, buf_len);
    *((unsigned int*)(buf + buf_len -4)) = htonl(hash);

    sent = sendto(sockfd, buf, buf_len, 0, (struct sockaddr*)&addr, addr_len);
    free(buf);
    return sent;
}
