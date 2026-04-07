#include "ms_connection.h"
#include "ms_session.h"
#include "addrport.h"

int connection_init(struct ms_connection * c, struct addrport *addr, unsigned short id)
{
    session_init(&c->session, id);
    c->addrport = *addr;
    return 0;
}
