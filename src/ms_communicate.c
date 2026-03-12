#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "ms_communicate.h"
#include "crc32.h"

int ms_crc32_check(char *buf, int buf_len)
{
    return CRC32_VALID != crc32(buf, buf_len);
}

int ms_send(int sockfd, struct ipv4_address *dst, unsigned short msg_type, char *msg, int msg_len)
{
    struct ms_msg_header h;
    h.type = msg_type;

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = dst->port;
    addr.sin_addr.s_addr = dst->address;
    socklen_t addr_len = sizeof(addr);
    
    int data_len = ms_header_len + msg_len;
    char* data = malloc(data_len);
    h.timestamp = get_timestamp();

    char* cur = data + ms_pack_header(&h, data);
    cur = msg;
    cur += msg_len - 1;

    int hash = 0; /* INSERT HASH FUNC HERE */

    *cur = hash;
    *(cur+5) = '\0';

    int sent;
    sent = sendto(sockfd, data, data_len, 0, (struct sockaddr*)&addr, addr_len);
    return sent;
}

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

    int src_hash = crc32_check(buf, received - 4), tmp_hash = *(buf + received - 4);
    if (src_hash != tmp_hash) {
        /* REQUEST PACKET AGAIN */
    }
    
    tmp.address = src.sin_addr.s_addr;
    tmp.port = src.sin_port;
    in_packet->addr = tmp;
    in_packet->data = buf + ms_header_len;
    in_packet->data_size = received - ms_header_len;
    ms_unpack_header(buf, &(in_packet->header));

    return received;
}
