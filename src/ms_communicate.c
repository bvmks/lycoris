#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "ms_headers.h"
#include "ms_communicate.h"
#include "crc32.h"
#include "ms_peer.h"

int ms_crc32_check(char *buf, int buf_len)
{
    return CRC32_VALID != crc32(buf, buf_len);
}

void ms_crc32_pack(char* buf, int buf_len)
{
    *((unsigned int*)(buf+buf_len-4)) = 0;
    *((unsigned int*)(buf+buf_len-4)) = crc32(buf, buf_len);
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
    
    ms_crc32_pack(buf, buf_len);

    sent = sendto(sockfd, buf, buf_len, 0, (struct sockaddr*)&dst, sizeof(dst));
    free(buf);
    return sent;
}

int ms_send(int sockfd, struct addrport* dst_addr, struct ms_packet* packet)
{
    return ms_send_raw(sockfd, dst_addr, &packet->header, packet->data, packet->data_size);
}

int ms_parse(char* inp, int inp_len, struct ms_header* header, char** data, int* data_len)
{
    int received, dlen;
    if (inp_len == 0) 
        return -1;
    *data_len = (inp_len - ms_header_size - ms_hash_size);
    dlen = *data_len;
    *data = malloc(dlen);
    ntoh_header(header, (struct ms_header*)inp);
    memcpy(inp+ms_header_size, *data, dlen);
    return 0;
}

int ms_recv(int sockfd, struct ms_received_packet *in_packet)
{
    /* TODO: DO */
    return NULL;
}

int ms_send_ctrl(int sockfd, struct ms_peer* p, unsigned short opt)
{
    int r;
    static struct ms_header h;
    h.s_id = p->session.id;
    h.seq = p->session.mask_conf.last_seq;
    h.type.type = mst_ctrl;
    h.type.opt = opt;
    r = ms_send_raw(sockfd, &p->addrport, &h, NULL, 0);
    ms_mask_inc(&p->session.mask_conf);
    return r;
}
