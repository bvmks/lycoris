#include <stdlib.h>
#include <string.h>

#include "ms_session.h"
#include "utils.h"

struct ms_udp_session* make_udp_session()
{
    struct ms_udp_session* s;
    s = malloc(sizeof(*s));
    s->state = ms_us_stub;
    s->side = 0;
    s->auth_type = 0;
    s->created_at = 0;
    s->last_rx = 0;
    s->last_tx = 0;
    s->remote_addr.addr = INADDR_ANY;
    s->remote_addr.port = -1;
    comctx_init(&s->comctx);
    return s;
}

void dispose_udp_session(struct ms_udp_session* s)
{
    free(s);
}


void ms_udp_session_init(struct ms_udp_session *s)
{

}

unsigned short generate_id(unsigned int key)
{
    unsigned short ip_key = (unsigned short)(key ^ (key >> 16));
    return ip_key ^ (unsigned short)rand();
}

void session_init(struct ms_session* session, unsigned short id)
{
    session->id = id;
    list_init(&session->recvd);
    list_init(&session->sent);
}
