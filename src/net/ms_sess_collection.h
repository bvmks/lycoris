#ifndef _MS_SESS_COLLECTION_H
#define _MS_SESS_COLLECTION_H

#include "ms_session.h"

struct ms_sesscoll_el {
    struct ms_udp_session *session;
    struct ms_sesscoll_el *next;
};

struct ms_sess_collection {
    struct ms_sesscoll_el *head;
    size_t count;
};

void ms_sess_coll_init(struct ms_sess_collection *coll);

int  ms_sess_coll_add(struct ms_sess_collection *coll, struct ms_udp_session *s);
void ms_sess_coll_remove(struct ms_sess_collection *coll, struct ms_udp_session *s);

struct ms_udp_session* ms_sess_coll_find(struct ms_sess_collection *coll, const struct addrport *addr);

void ms_sess_coll_cleanup(struct ms_sess_collection *coll);

int mss_sess_get_num_of_el(struct ms_sess_collection* coll);

#endif
