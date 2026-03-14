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

static void ntoh_header(struct ms_header* dst, struct ms_header* src) {
    dst->type = src->type;
    dst->s_id = ntohs(src->s_id);
    dst->seq = ntohs(src->seq);
}

int ms_send_raw(int sockfd, struct addrport *dst_addr,
                struct ms_header *header, 
                const char *data, int data_len)
{
    char *buf;
    int buf_len, sent;
    struct sockaddr_in dst;

    dst.sin_family = AF_INET;
    dst.sin_addr.s_addr = htonl(dst_addr->addr);
    dst.sin_port = htons(dst_addr->port);
    
    buf_len = ms_header_size + data_len + hash_size;
    buf = malloc(buf_len);
    hton_header((struct ms_header*)buf, header);
    
    if(data)
        memcpy(buf + ms_header_size, data, data_len);
    
    *((unsigned int*)(buf + buf_len - hash_size)) = 0;
    *((unsigned int*)(buf + buf_len - hash_size)) = htonl(crc32(buf, buf_len));

    sent = sendto(sockfd, buf, buf_len, 0, (struct sockaddr*)&dst, sizeof(dst));
    free(buf);
    return sent;
}

int ms_send(int sockfd, struct addrport* dst_addr, struct ms_packet* packet)
{
    return ms_send_raw(sockfd, dst_addr, &packet->header, packet->data, packet->data_size);
}


int ms_recv(int sockfd, struct ms_received_packet *in_packet)
{
    int received;
    char* buf = malloc(dgram_max_size);
    char* buf_end = buf + received;
    struct sockaddr_in src;
    socklen_t src_len = sizeof(src);
    received = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&src, &src_len);
    
    if (received == 0) {
        in_packet->src_addr.addr            = ntohl(src.sin_addr.s_addr);
        in_packet->src_addr.port            = ntohs(src.sin_port);
        in_packet->packet.data              = NULL;
        in_packet->packet.data_size         = 0;
        in_packet->packet.header.s_id       = 0;
        in_packet->packet.header.seq        = 0;
        in_packet->packet.header.type.type  = mst_err;
        in_packet->packet.header.type.ctrl  = msc_err;
        return 0;
    }
    else if (received == -1) {
        return -1;
    }

    if (ms_crc32_check(buf, received)) {
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

