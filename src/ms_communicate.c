#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "ms_communicate.h"
#include "ms_headers.h"
#include "crc32.h"

int ms_crc32_check(const char *buf, int buf_len)
{
    return CRC32_VALID != crc32(buf, buf_len);
}

void ms_crc32_pack(char* buf, int buf_len)
{
    *((unsigned int*)(buf+buf_len-4)) = 0;
    *((unsigned int*)(buf+buf_len-4)) = crc32(buf, buf_len);
}

int ms_send_raw(int sockfd,
                const struct addrport *dst_addr,
                const struct ms_header *header, 
                const char *data,
                int data_len)
{
    static char buf[dgram_max_size];
    int msg_len, sent;
    struct sockaddr_in dst;

    msg_len = ms_header_size + data_len + ms_hash_size;
    if (msg_len > dgram_max_size)
        return ms_ce_msg_too_long;
    addrport2sockaddr_in(&dst, dst_addr);
    hton_header((struct ms_header*)buf, header);
    if(data)
        memcpy(buf + ms_header_size, data, data_len);
    ms_crc32_pack(buf, msg_len);
    sent = sendto(sockfd, buf, msg_len, 0, (struct sockaddr*)&dst, sizeof(dst));
    return sent;
}

int ms_send_packet(int sockfd,
                   const struct addrport* dst_addr,
                   const struct ms_packet* packet)
{
    return ms_send_raw(sockfd, dst_addr, &packet->header, packet->data, packet->data_size);
}

int ms_parse(const char* inp,
             int inp_len,
             struct ms_header* header,
             char** data,
             int* data_len)
{
    if (inp_len == 0) 
        return -1;
    *data_len = (inp_len - ms_header_size - ms_hash_size);
    *data = malloc(*data_len);
    ntoh_header(header, (struct ms_header*)inp);
    memcpy(*data, inp+ms_header_size, *data_len);
    return 0;
}

