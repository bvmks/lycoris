#include <stdlib.h>
#include <string.h>

#include "ms_sess_collection.h"

void ms_sess_coll_init(struct ms_sess_collection *coll) {
    coll->head = NULL;
    coll->count = 0;
}

int ms_sess_coll_add(struct ms_sess_collection *coll, struct ms_udp_session *s) {
    struct ms_sesscoll_el *node = malloc(sizeof(*node));
    if (!node) return -1;

    node->session = s;
    node->next = coll->head;
    coll->head = node;
    coll->head->next = NULL;
    coll->count++;
    return 0;
}

struct ms_udp_session* ms_sess_coll_find(struct ms_sess_collection *coll, const struct addrport *addr) {
    struct ms_sesscoll_el *curr;
    curr = coll->head;
    while (curr) {
        if (addrport_equal(&curr->session->remote_addr, addr)) {
            return curr->session;
        }
        curr = curr->next;
    }
    return NULL;
}

void ms_sess_coll_remove(struct ms_sess_collection *coll, struct ms_udp_session *s) {
    struct ms_sesscoll_el *entry;
    struct ms_sesscoll_el **pp = &coll->head;
    while (*pp) {
        entry = *pp;
        if (entry->session == s) {
            *pp = entry->next;
            free(entry);
            coll->count--;
            return;
        }
        pp = &entry->next;
    }
}

void ms_sess_coll_cleanup(struct ms_sess_collection *coll) {
    struct ms_sesscoll_el *next;
    struct ms_sesscoll_el *curr = coll->head;
    while (curr) {
        next = curr->next;
        if (curr->session->hctx) free(curr->session->hctx);
        free(curr->session);
        
        free(curr);
        curr = next;
    }
    coll->head = NULL;
    coll->count = 0;
}


int mss_sess_get_el_num(struct ms_sess_collection* coll)
{
    return coll->count;
}
