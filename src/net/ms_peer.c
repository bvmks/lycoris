#include <stdlib.h>
#include <string.h>

#include "ms_peer.h"

struct ms_peer* make_udp_session()
{
    struct ms_peer* s;
    s = malloc(sizeof(*s));
    memset(s->id, 0, node_id_size);
    memset(s->public_key, 0, node_id_size);
    s->state = ms_us_stub;
    s->init_assoc = 0;
    s->created_at = 0;
    s->last_rx = 0;
    s->last_tx = 0;
    s->ip = INADDR_ANY;
    s->port = -1;
    comctx_init(&s->comctx);
    return s;
}

void dispose_udp_session(struct ms_peer* s)
{
    free(s);
}


void ms_udp_session_init(struct ms_peer *s)
{

}

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


void ms_peer_coll_init(struct ms_peer_collection *coll) {
    coll->head = NULL;
    coll->count = 0;
}

int ms_peer_coll_add(struct ms_peer_collection *coll, struct ms_peer *s) {
    struct ms_peer_el *node = malloc(sizeof(*node));
    if (!node) return -1;

    node->peer = s;
    node->next = coll->head;
    coll->head = node;
    coll->count++;
    return 0;
}

struct ms_peer* ms_peer_coll_find(struct ms_peer_collection *coll, unsigned int ip, unsigned short port) {
    struct ms_peer_el *curr;
    curr = coll->head;
    while (curr) {
        if (curr->peer->ip == ip && curr->peer->port == port) {
            return curr->peer;
        }
        curr = curr->next;
    }
    return NULL;
}

void ms_peer_coll_remove(struct ms_peer_collection *coll, struct ms_peer *s) {
    struct ms_peer_el **pp = &coll->head;
    while (*pp) {
        struct ms_peer_el *entry = *pp;
        if (entry->peer == s) {
            *pp = entry->next;
            dispose_udp_session(entry->peer);
            free(entry); 
            coll->count--;
            return;
        }
        pp = &entry->next;
    }
}

void ms_peer_coll_cleanup(struct ms_peer_collection *coll) {
    struct ms_peer_el *next;
    struct ms_peer_el *curr = coll->head;
    while (curr) {
        next = curr->next;
        dispose_udp_session(curr->peer);
        free(curr);
        curr = next;
    }
    coll->head = NULL;
    coll->count = 0;
}


int mss_sess_get_el_num(struct ms_peer_collection* coll)
{
    return coll->count;
}
