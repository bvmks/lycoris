#include <string.h>

#include "ms_hl.h"

int ms_send(int sockfd,
            struct ms_peer* p,
            unsigned short type,
            unsigned short opt,
            const char* buf,
            int buf_len)
{
    int r;
    static struct ms_header h;

    h.s_id = p->session.id;
    h.seq = p->session.sent.mask.last_seq;
    h.type.type = type;
    h.type.opt = opt;
    r = ms_send_raw(sockfd, &p->addrport, &h, buf, buf_len);
    if(r != -1)
        ms_mask_inc(&p->session.sent.mask);
    return r;
}

int ms_send_ctrl(int sockfd, struct ms_peer* p, unsigned short opt)
{
    return ms_send(sockfd, p, mst_ctrl, opt, NULL, 0);
}

int ms_recv_packet(int sockfd, struct ms_received_packet* p)
{
    static char buf [dgram_max_size];
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    int recvd;
    recvd = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &addr_len);
    if(recvd == -1)
        return recvd;
    ms_parse(buf, sizeof(buf), &p->packet.header, &p->packet.data, &p->packet.data_size);
    return recvd;
}
