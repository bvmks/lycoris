#include <stdlib.h>

#include "ms_session.h"

unsigned short generate_id(unsigned int key)
{
    unsigned short ip_key = (unsigned short)(key ^ (key >> 16));
    return ip_key ^ (unsigned short)rand();
}

void mss_init_session(struct ms_session* session, unsigned int skey)
{
    session->id = generate_id(skey);
    mss_init_list(&session->recvd);
    mss_init_list(&session->sent);
}
