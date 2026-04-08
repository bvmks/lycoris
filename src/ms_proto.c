#include <string.h>

#include "ms_proto.h"

static int _ms_send_post_seq(int sockfd,
            struct ms_connection* c,
            unsigned short type,
            unsigned short opt,
            unsigned short seq,
            const char* buf,
            int buf_len)
{
    int r;
    static struct ms_post_header h;
    h.s_id = c->session.id;
    h.seq = seq;
    h.type.t = type;
    h.type.o = opt;
    r = ms_send_raw(sockfd, &c->addrport, (struct ms_header*)&h, buf, buf_len);
    return r;
}

int ms_send_post(int sockfd,
            struct ms_connection* c,
            const char* buf,
            int buf_len)
{
    int r;
    r = _ms_send_post_seq(sockfd, c, mst_post, 0, c->session.sent.mask.last_seq, buf, buf_len);
    if(r != -1)
        ms_mask_inc(&c->session.sent.mask);
    return r;
}

int ms_send_ctrl(int sockfd, struct ms_connection* c, unsigned short opt)
{
    return ms_send_post(sockfd, c, NULL, 0);
}

int ms_recv_packet(int sockfd, struct ms_received_packet* p)
{
    static char buf [dgram_max_size];
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    int recvd;
    recvd = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &addr_len);
    sockaddr_in2addrport(&p->src_addr, &addr );
    if(recvd == net_error)
        return recvd;
    // else if(ms_crc32_check(buf, sizeof(buf))) {
    //     return pkg_damaged;
    // }
    ms_parse(buf, sizeof(buf), &p->packet.header, &p->packet.data, &p->packet.data_size);
    return recvd;
}

int ms_send_confirm(int sockfd, struct ms_connection* c, unsigned short seq)
{
    return _ms_send_post_seq(sockfd, c, mst_ctrl, msc_conf, seq, NULL, 0);
}

