#include <stdlib.h>
#include <string.h>

#include "ms_kndb.h"
#include "ms_nodecfg.h"

struct ms_known_node {
    unsigned char id[node_id_size];
    unsigned char pubkey[public_key_size];
    struct ms_known_node* next;
};

struct known_nodes_db {
    struct ms_known_node *first;
};


struct known_nodes_db* make_kndb(struct ms_node_cfg* cfg)
{
    struct known_nodes_db* res;
    res = malloc(sizeof(*res));
    memset(res, 0 , sizeof(*res));
    return res;
}

static struct ms_known_node* make_known_node()
{
    struct ms_known_node* res;
    res = malloc(sizeof(*res));
    memset(res, 0 ,sizeof(*res));
    return res;
}

static void enlist_known_node(struct known_nodes_db* db, struct ms_known_node* node)
{
    node->next = db->first;
    db->first = node;
}

int test_init_kndb(struct known_nodes_db* db, unsigned char* node1, unsigned char* pub1)
{
    struct ms_known_node* n;

    n = make_known_node();
    memcpy(n->id, node1, node_id_size);
    memcpy(n->pubkey, pub1, public_key_size);

    enlist_known_node(db, n);

    return 1;
}


struct known_nodes_db* load_kndb(struct ms_node_cfg* cfg)
{
    return make_kndb(cfg);
}


int kndb_get_node(struct known_nodes_db* db,
                  const unsigned char* node_id,
                  unsigned char* pubkey)
{
    struct ms_known_node* p;
    for(p = db->first; p; p = p->next) {
        if(memcmp(p->id, node_id, node_id_size) == 0) {
            memcpy(pubkey, p->pubkey, public_key_size);
            return kndb_res_success;
        }
    }
    return kndb_res_node_unknown;
}
