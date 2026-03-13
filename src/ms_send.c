#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "ms_communicate.h"
#include "crc32.h"

static void hton_header(struct ms_header* dst, struct ms_header* src) {
    dst->type = src->type;
    dst->s_id = htons(src->s_id);
    dst->seq = htons(src->seq);
}

int ms_send(int sockfd, struct sockaddr_in* dst_addr, struct ms_packet* packet)
{
    char *buf, *cur;
    int buf_len, sent;
    unsigned int hash;
    
    buf_len = sizeof(struct ms_header) + packet->data_size + 4;
    buf = malloc(buf_len);
    hton_header((struct ms_header*)buf, &packet->header);
    
    if(packet->data)
        memcpy(buf + sizeof(struct ms_header), packet->data, packet->data_size);
    
    *((unsigned int*)(buf + buf_len -4)) = 0;
    *((unsigned int*)(buf + buf_len -4)) = htonl(crc32(buf, buf_len));

    sent = sendto(sockfd, buf, buf_len, 0, (struct sockaddr*)dst_addr, sizeof(*dst_addr));
    free(buf);
    return sent;
}
