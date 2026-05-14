#include <stdlib.h>
#include <string.h>

#include "addrcol.h"
#include "addrport.h"
#include "unitable.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

enum {incaddr_col_init_map_size = 512};


static int ic_match_ipport(const void* a, const void* b)
{
    return !memcmp(a, b, 6);
}

static unsigned long long ic_hash_ipport(const void* ipport)
{
    /* an some 64-bit FNV-1a (stolen from AI, i dont give a fuck what it is...)*/
    /* at least it mixes bytes +- evenly */
    const unsigned char* key = ipport;
    unsigned long long hash = 14695981039346656037ULL;
    const unsigned long long prime = 1099511628211ULL;

    for (int i = 0; i < 6; i++) {
        hash ^= key[i];
        hash *= prime;
    }

    return hash;
}

static void ic_cr (struct ut_node* n, const void* key)
{
    struct addr_item *item = malloc(sizeof(*item));
    /* it's a bit stupid that we creating it here, and only set key
    but at least there's no additionall malloc for key*/
    item->the_master = NULL;
    n->userdata = item;
    n->key = item->key;
}

void addrcoll_init(struct addr_collection* coll, long long starttime, long long timeout)
{
    unitable_init(&coll->map,
                  incaddr_col_init_map_size,
                  &ic_hash_ipport,
                  &ic_match_ipport, 
                  &ic_cr,
                  NULL);
    coll->starttime = starttime;
    coll->curtime = 0;
    coll->timeout = timeout;
    coll->last = NULL;
    coll->first = NULL;
}

int addrcoll_update(struct addr_collection* coll, long long ct)
{
    int newtm = ct - coll->starttime;
    if(coll->curtime == newtm)
        return 0;
    coll->curtime = newtm;
    return 1; 
}


struct addr_item* addrcoll_find(struct addr_collection* coll,
                                    unsigned int ip, unsigned short port,
                                    int add)
{
    unsigned char key[ipport_len];
    ipport2mem(key, ip, port);

    if(add) {
        struct addr_item* item = *unitable_provide(&coll->map, key);
        if(item->the_master) {
            return item;
        } else {
            item->next = NULL;
            item->prev = coll->last;
            if(coll->last)
                coll->last->next = item;
            else
                coll->first = item;
            coll->last = item;

            item->the_master = coll;
            memcpy(item->key, key, ipport_len);
            item->timemark = coll->curtime;
            item->userdata = NULL;
            item->timeout_hook = NULL;
            item->destruction_hook = NULL;
            return item;
        }
    } else {
        void *p = unitable_get(&coll->map, key);
        return p;
    }

}


static void do_remove_from_list(struct addr_collection *coll,
                                struct addr_item *item)
{
    if(!item->prev && !item->next && coll->first != item)
        return;
    if(item->prev)
        item->prev->next = item->next;
    else
        coll->first = item->next;
    if(item->next)
        item->next->prev = item->prev;
    else
        coll->last = item->prev;
    item->next = NULL;
    item->prev = NULL;
}

void addritem_reset(struct addr_item *item)
{
    struct addr_collection *coll = item->the_master;

    do_remove_from_list(coll, item);
    item->timemark = coll->curtime;
    item->prev = coll->last;
    if(coll->last)
        coll->last->next = item;
    else
        coll->first = item;
    coll->last = item;
}

struct addr_item* addrcoll_permadd(struct addr_collection *coll,
                                       unsigned int ip, unsigned short port)
{
    unsigned char key[ipport_len];
    struct addr_item *item;

    ipport2mem(key, ip, port);

    item = *unitable_provide(&coll->map, key);
    if(item->the_master) {
        do_remove_from_list(coll, item);
    } else {
        item->next = NULL;
        item->prev = NULL;

        item->the_master = coll;
        memcpy(item->key, key, sizeof(key));
        item->timemark = coll->curtime;
        item->userdata = NULL;
        item->timeout_hook = NULL;
        item->destruction_hook = NULL;
    }
    return item;
}

void addritem_getaddr(struct addr_item *item,
                        unsigned int *ip, unsigned short *port)
{
    mem2ipport(item->key, ip, port);
}

void addritem_remove(struct addr_item *item)
{
    struct addr_collection *coll = item->the_master;

    do_remove_from_list(coll, item);
    unitable_delete(&coll->map, item->key);

    if(item->destruction_hook)
        (*item->destruction_hook)(item);

    free(item);
}


void addrcoll_process(struct addr_collection *coll)
{
    long long curtime, min2keep;

    curtime = coll->curtime;
    min2keep = curtime - coll->timeout;

    while(coll->first && coll->first->timemark < min2keep) {
        if(coll->first->timeout_hook)
            (*coll->first->timeout_hook)(coll->first);
        else
            addritem_remove(coll->first);
    }
}

