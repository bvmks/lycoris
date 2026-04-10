#include <stdlib.h>

#include "ms_session.h"

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
