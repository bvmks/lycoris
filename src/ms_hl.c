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
    h.seq = p->session.mask_conf.last_seq;
    h.type.type = type;
    h.type.opt = opt;
    r = ms_send_raw(sockfd, &p->addrport, &h, buf, buf_len);
    ms_mask_inc(&p->session.mask_conf);
    return r;
}

int ms_send_ctrl(int sockfd, struct ms_peer* p, unsigned short opt)
{
    return ms_send(sockfd, p, mst_ctrl, opt, NULL, 0);
}
