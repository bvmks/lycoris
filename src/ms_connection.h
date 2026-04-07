#ifndef _MS_PEER_H
#define _MS_PEER_H

#include "ms_session.h"
#include "addrport.h"

struct ms_connection {
    struct ms_session session;
    struct addrport addrport;
};

/*
 * initializes session and peer
 * with provided addrport
 * returns 0 if all gucci
*/
int connection_init(struct ms_connection*, struct addrport* addr, unsigned short id);

#endif
