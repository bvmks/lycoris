#include <stdlib.h>
#include <string.h>

#include "ms_session.h"
#include "utils.h"


struct ms_udp_session* make_udp_session()
{
    struct ms_udp_session* s;
    s = malloc(sizeof(*s));
    s->created_at = 0;
    s->last_activity = 0;
    s->hctx = NULL;
    ms_nonce_init_rand(&s->nonce);
    s->remote_addr.addr = 0;
    s->remote_addr.port = 0;
    memset(s->rx_key, 0 , rx_key_size);
    memset(s->tx_key, 0 , tx_key_size);
    return s;
}

void dispose_udp_session(struct ms_udp_session* s);


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
