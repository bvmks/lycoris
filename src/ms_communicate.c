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
    
    buf_len = ms_header_size + data_len + ms_hash_size;
    buf = malloc(buf_len);
    hton_header((struct ms_header*)buf, header);
    
    if(data)
        memcpy(buf + ms_header_size, data, data_len);
    
    *((unsigned int*)(buf + buf_len - ms_hash_size)) = 0;
    *((unsigned int*)(buf + buf_len - ms_hash_size)) = htonl(crc32(buf, buf_len));

    sent = sendto(sockfd, buf, buf_len, 0, (struct sockaddr*)&dst, sizeof(dst));
    free(buf);
    return sent;
}

int ms_send(int sockfd, struct addrport* dst_addr, struct ms_packet* packet)
{
    return ms_send_raw(sockfd, dst_addr, &packet->header, packet->data, packet->data_size);
}

int ms_parse(char* inp, int inp_len, struct ms_header* header, char* data, int* data_len)
{
    int received, dlen;
    if (inp_len == 0) 
        return -1;
    *data_len = (inp_len - ms_header_size - ms_hash_size);
    dlen = *data_len;
    data = malloc(dlen);
    ntoh_header(header, (struct ms_header*)inp);
    memcpy(inp+ms_header_size, data, dlen);
    return 0;
}

int ms_recv(int sockfd, struct ms_received_packet *in_packet)
{
    return NULL;
}

