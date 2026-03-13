#include <time.h>

#include "ms_session.h"

unsigned short generate_id() 
{
    return time(NULL);
}

void mss_init_session(struct ms_session* session)
{
    session->id = generate_id();
    session->lseq = 0;
    session->rmask = 0;
}

int mss_check_dup(struct ms_header* header, struct ms_session* session)
{
    if(session->id != header->s_id)
        return 0;
    else if(header->seq == session->lseq)
        return 1;
    else if(header->seq > session->lseq)
        return 0;
    else if(session->lseq - header->seq > sizeof(session->rmask)*8 )
        return 0;
    else 
        return session->rmask & (1 << (session->lseq - header->seq - 1));
}

int mss_add_recvd(struct ms_header* header, struct ms_session* session) 
{
    int chk = mss_check_dup(header, session);
    if(chk)
        return 1;
    if(header->seq > session->lseq) {
        session->rmask = (session->rmask << 1) | 1;
        session->rmask <<= header->seq - session->lseq - 1;
        session->lseq = header->seq;
    }
    return 0;
}
