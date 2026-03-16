#include "ms_peer.h"
#include "ms_session.h"
#include "addrport.h"

int ms_peer_init(struct addrport* ap, struct ms_peer* p)
{
    mss_init_session(&p->session, ap->addr);
    p->addrport = *ap;
    return 0;
}
