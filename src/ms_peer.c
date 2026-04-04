#include "ms_peer.h"
#include "ms_session.h"
#include "addrport.h"

int ms_peer_init(struct ms_peer* p, struct addrport* ap, unsigned short id)
{
    session_init(&p->session, id);
    p->addrport = *ap;
    return 0;
}
