#include "unitable.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>




void unitable_init(struct unitable* t,
                   unsigned long long capacity, 
                   ut_hash_cb hash_cb, 
                   ut_match_cb match_cb, 
                   ut_create_hook cr_h, 
                   ut_destruct_hook dest_h)
{
    t->buckets = calloc(capacity, sizeof(struct ut_node*));

    t->capacity = capacity;
    t->nodes = 0;
    t->hash = hash_cb;
    t->match = match_cb;
    t->creation_hook = cr_h;
    t->destruction_hook = dest_h;
}

struct unitable* make_unitable(unsigned long long capacity, 
                               ut_hash_cb hash_cb, 
                               ut_match_cb match_cb, 
                               ut_create_hook cr_h, 
                               ut_destruct_hook dest_h)
{
    struct unitable* t;
    t = malloc(sizeof(*t));
    t->buckets = calloc(capacity, sizeof(struct ut_node*));

    t->capacity = capacity;
    t->nodes = 0;
    t->hash = hash_cb;
    t->match = match_cb;
    t->creation_hook = cr_h;
    t->destruction_hook = dest_h;
    return t;
}

#if 0
/* not needed */
void** unitable_insert(struct unitable* t, const void* key, void* value) {
    size_t idx; 
    struct ut_node* n; 
    struct ut_node* nn;

    idx = t->hash(key) % t->capacity;
    n = t->buckets[idx];

    while (n) {
        if (t->match(n->key, key)) {
            if (t->free_val) t->free_val(n->value);
            n->value = value;
            return &(n->value);
        }
        n = n->next;
    }

    nn = (struct ut_node*)malloc(sizeof(struct ut_node));

    nn->key = t->make_key_copy(key);
    nn->value = value;
    nn->next = t->buckets[idx];
    t->buckets[idx] = nn;
    t->fill++;
    return &(nn->value);
}
#endif

static struct ut_node* traverse(struct unitable* t, const void* key, int mk)
{
    unsigned long long idx; 
    struct ut_node** pp; 
    idx= t->hash(key) % t->capacity;
    pp = &(t->buckets[idx]);

    for(; *pp; pp = &((*pp)->next)) {
        if (t->match((*pp)->key, key))
            return *pp;
    }
    if(mk){
        *pp = malloc(sizeof(struct ut_node));
        (*pp)->userdata = NULL;
        (*pp)->key = NULL;
        (*pp)->next = NULL;
        if(t->creation_hook)
            t->creation_hook(*pp, key);
        t->nodes++;
        return *pp;
    }
    return NULL;
}

void* unitable_get(struct unitable* t, const void* key)
{
    struct ut_node* n = traverse(t, key, 0);
    if(n) {
        return n->userdata;
    }
    return NULL;
}


void** unitable_provide(struct unitable* t, const void* key)
{
    struct ut_node* node; 
    node = traverse(t, key, 1);
    return &(node->userdata);
}

int unitable_delete(struct unitable* t, const void* key)
{
    if (t->nodes == 0) return -1;

    unsigned long long idx = t->hash(key) % t->capacity;
    struct ut_node** pp = &t->buckets[idx];

    while (*pp) {
        struct ut_node* entry = *pp;

        if (t->match(entry->key, key)) {
            *pp = entry->next;
            if(t->destruction_hook)
                t->destruction_hook(entry);
            free(entry);
            t->nodes--;
            return 0;
        }
        pp = &entry->next;
    }
    return -1;
}

unsigned long long get_capacity(struct unitable* t)
{
    return t->capacity;
}

unsigned long long get_nodes_num(struct unitable* t)
{
    return t->nodes;
}

int unitable_realloc(struct unitable* t, unsigned long long new_capacity)
{
    unsigned long long i;
    struct ut_node** new_buckets; 

    if (new_capacity <= 0) return -1;
    new_buckets = calloc(new_capacity, sizeof(struct ut_node*));

    for (i = 0; i < t->capacity; i++) {
        struct ut_node* n = t->buckets[i];
        while (n) {
            struct ut_node* next = n->next;
            unsigned long long new_idx = t->hash(n->key) % new_capacity;
            n->next = new_buckets[new_idx];
            new_buckets[new_idx] = n;

            n = next;
        }
    }

    free(t->buckets);
    t->buckets = new_buckets;
    t->capacity = new_capacity;

    return 0;
}

void dispose_unitable(struct unitable* t)
{
    unsigned long long i;
    for (i = 0; i < t->capacity; i++) {
        struct ut_node* n = t->buckets[i];
        while (n) {
            struct ut_node* pn = n;
            n = n->next;
            t->destruction_hook(pn);
            free(pn);
        }
    }
    free(t->buckets);
    free(t);
}

#ifdef _UNITABLE_TEST

enum {test_table_size = 512};

static unsigned long long test_hash(const void* src)
{
    /* 64-bit FNV-1a (stolen from AI, i dont give a fuck what it is...)*/
    const unsigned char* key = src;
    unsigned long long hash = 14695981039346656037ULL;
    const unsigned long long prime = 1099511628211ULL;

    for (int i = 0; i < strlen(src); i++) {
        hash ^= key[i];
        hash *= prime;
    }

    return hash;
}

static int test_match(const void* a, const void* b)
{
    return !strcmp(a, b);
}

static void test_cr(struct ut_node* n, const void* key) 
{
    n->key = strdup(key);
}

static void test_del(struct ut_node* n) 
{
    if(n->key)
        free(n->key);
}

static struct unitable* make_test_table()
{
    return make_unitable(test_table_size, 
                         &test_hash,
                         &test_match,
                         &test_cr,
                         &test_del);
}

static int test_addpair(struct unitable* t, const char* key, void* value)
{
    void** res;
    res = unitable_provide(t, key);
    *res = value;
    printf("[TEST] adding %s - %s\n", key, (char*)value);
    return 0;
}

static int test_getpair(struct unitable* t, const char* key, const char* expected)
{
    void* res = unitable_get(t, key);
    printf("[TEST] testing find %s\n", key);

    if (res == NULL && expected == NULL) {
        printf("[SUCCESS] correctly not found\n");
        return 0;
    }

    if (res != NULL && expected != NULL && strcmp(res, expected) == 0) {
        printf("[SUCCESS] found %s\n", (char*)res);
        return 0;
    }

    printf("[ERROR] found %s, expected %s\n", 
           res ? (char*)res : "NULL", 
           expected ? expected : "NULL");
    return 1;
}

static int test_delpair(struct unitable* t, const char* key, int expect)
{
    int res;
    printf("[TEST] testing deleting %s\n", key);
    res = unitable_delete(t, key);
    if(res && expect)
    {
        printf("[ERROR] expected to have value\n");
        return 1;
    }
    else if (!res && !expect) {
        printf("[ERROR] didn't expected to have value\n");
        return 1;
    }
    printf("[SUCCESS]\n");
    return 0;
}

int main(int argc, char *argv[])
{
    int err;
    int full_err = 0;
    struct unitable* t;
    t = make_test_table();
    
    printf("running unitable tests...\n");
    err = test_addpair(t, "bebra", "zebra");
    full_err = full_err ? 1 : err;
    err = test_addpair(t, "cobra", "gobra");
    full_err = full_err ? 1 : err;
    err = test_addpair(t, "schvabra", "kadabra");
    full_err = full_err ? 1 : err;
    err = test_delpair(t, "kaput", 0);
    full_err = full_err ? 1 : err;

    err = test_getpair(t, "cobra", "gobra");
    full_err = full_err ? 1 : err;
    err = test_getpair(t, "schvabra", "kadabra");
    full_err = full_err ? 1 : err;

    err = test_addpair(t, "cobra", "bibizyan");
    full_err = full_err ? 1 : err;
    err = test_getpair(t, "cobra", "bibizyan");
    full_err = full_err ? 1 : err;

    err = test_delpair(t, "schvabra", 1);
    full_err = full_err ? 1 : err;

    err = test_getpair(t, "bebra", "zebra");
    full_err = full_err ? 1 : err;
    err = test_getpair(t, "cobra", "bibizyan");
    full_err = full_err ? 1 : err;
    err = test_getpair(t, "schvabra", NULL);
    full_err = full_err ? 1 : err;


    printf("reallocing table\n");
    unitable_realloc(t, 128);

    err = test_delpair(t, "kaput", 0);
    full_err = full_err ? 1 : err;

    err = test_delpair(t, "schvabra", 0);
    full_err = full_err ? 1 : err;

    err = test_getpair(t, "bebra", "zebra");
    full_err = full_err ? 1 : err;
    err = test_getpair(t, "cobra", "bibizyan");
    full_err = full_err ? 1 : err;
    err = test_getpair(t, "schvabra", NULL);
    full_err = full_err ? 1 : err;
    err = test_getpair(t, "kaput", NULL);
    full_err = full_err ? 1 : err;

    if(full_err) {
        printf("[SAGDE] unitable tests failed\n");
        return 1;
    }
    else {
        printf("[GOOD] unitable tests success\n");
        return 0;
    }
}

#endif


