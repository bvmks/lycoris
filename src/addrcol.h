#ifndef _MS_INADDR_COL_H
#define _MS_INADDR_COL_H

#include "unitable.h"

struct addr_collection;

enum {ipport_len = 6};

struct addr_item {
    struct addr_collection* the_master;
    struct addr_item *prev, *next;

    unsigned char key[ipport_len];

    long long timemark;
    void* userdata;
    void (*timeout_hook)(struct addr_item*);
    void (*destruction_hook)(struct addr_item*);
};

struct addr_collection {
    /* TODO: probably for map must use tree instead of hash table
             will fix it later (i hope)
    */
    struct unitable map;
    struct addr_item *first, *last;
    long long starttime;
    long long curtime, timeout;
};

void addrcoll_init(struct addr_collection* coll,
                     long long starttime, long long timeout);

int addrcoll_update(struct addr_collection* coll,
                           long long current_time);

void addritem_reset(struct addr_item *item);


struct addr_item* addrcoll_find(struct addr_collection* coll,
                                    unsigned int ip, unsigned short port,
                                    int add);


struct addr_item* addrcoll_permadd(struct addr_collection* coll,
                                       unsigned int ip, unsigned short port);

/*  call timeout_hooks or removes all timedout items until first non-timedout
    timeout_hook must remove or reset item
*/
void addrcoll_process(struct addr_collection *coll);

void addritem_getaddr(struct addr_item *item,
                        unsigned int *ip, unsigned short *port);

/* resets timemark and puts to head of the list */
void inaddritem_reset(struct addr_item *item);

void addritem_remove(struct addr_item *item);


#endif
