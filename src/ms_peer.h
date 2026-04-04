#ifndef _MS_PEER_H
#define _MS_PEER_H

#include "ms_session.h"
#include "addrport.h"

struct ms_peer {
    struct ms_session session;
    struct addrport addrport;
};

/*
 * initializes session and peer
 * with provided addrport
 * returns 0 if all gucci
*/
int peer_init(struct ms_peer*, struct addrport* addr, unsigned short id);

#endif
