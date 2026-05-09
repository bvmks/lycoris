#ifndef _UNIVERSAL_TABLE_H
#define _UNIVERSAL_TABLE_H

/*  
    this header provides universal hash table 
    with 'list' collision solution
*/

struct unitable;
struct ut_node;

/* used to calculate hash of key */
typedef unsigned long long (*ut_hash_cb)(const void* key);

/* boolean, may be called several times during search */
typedef int (*ut_match_cb)(const void* n, const void* key);

/* node creation hook */
typedef void (*ut_create_hook)(struct ut_node* n, const void* key);

/* node destruction hook, IT SHOULDN'T free node itself */ 
/* only free all that may be was allocated inside */
typedef void (*ut_destruct_hook)(struct ut_node* n);

struct ut_node {
    void* key;
    void* userdata;
    struct ut_node* next;
};

struct unitable {
    struct ut_node** buckets;
    unsigned long long capacity; /* number of buckets */
    unsigned long long nodes;    /* number of nodes */
                                 /* of cours may exceed capacity */
    ut_hash_cb hash;
    ut_match_cb match;
    ut_create_hook creation_hook;
    ut_destruct_hook destruction_hook;
};




void unitable_init(struct unitable* t,
                   unsigned long long capacity, 
                   ut_hash_cb hash_cb, 
                   ut_match_cb match_cb, 
                   ut_create_hook cr_h, 
                   ut_destruct_hook dest_h);

struct unitable* make_unitable(unsigned long long capacity, 
                               ut_hash_cb hash_cb,
                               ut_match_cb match_cb, 
                               ut_create_hook cr_cb, 
                               ut_destruct_hook del_cb);

void* unitable_get(struct unitable* t, const void* key);
void** unitable_provide(struct unitable* t, const void* key);
int unitable_delete(struct unitable* t, const void* key);

/* creates buckets[new_capacity], then recalculates hash and moves nodes to new buckets */
/* it can shrink table */
int unitable_realloc(struct unitable* t, unsigned long long new_capacity);

void dispose_unitable(struct unitable* t);

unsigned long long get_capacity(struct unitable* t);
unsigned long long get_nodes_num(struct unitable* t);

#endif
