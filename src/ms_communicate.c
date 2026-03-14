#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "ms_communicate.h"
#include "crc32.h"

int ms_crc32_check(char *buf, int buf_len)
{
    return CRC32_VALID != crc32(buf, buf_len);
}

static void hton_header(struct ms_header* dst, struct ms_header* src) {
    dst->type = src->type;
    dst->s_id = htons(src->s_id);
    dst->seq = htons(src->seq);
}

int ms_send(int sockfd, struct addrport* dst_addr, struct ms_packet* packet)
{
    char *buf;
    int buf_len, sent;
    
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

int ms_recv(int sockfd, struct ms_received_packet *in_packet)
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

