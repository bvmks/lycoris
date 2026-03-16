#ifndef _MS_PEER_H
#define _MS_PEER_H

#include "ms_session.h"
#include "addrport.h"

struct ms_peer {
    struct ms_session session;
    struct addrport addrport;
};



#endif
