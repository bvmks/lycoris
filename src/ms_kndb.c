#include <stdlib.h>
#include <string.h>

#include "ms_kndb.h"
#include "message.h"
#include "ms_nodecfg.h"
#include "hexdata.h"

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

int test_init_kndb(struct known_nodes_db* db)
{
    struct ms_known_node* n;
    int res;
    char* node1 = "a473aa3ee7ecfb25daee";
    char* pub1 = "71821bc554c753efba6a2a5ba4fd2ecd5dd39520470ca473aa3ee7ecfb25daee";

    n = make_known_node();
    res = hexstr2data(n->id, node_id_size, node1);
    if(res != node_id_size) {
        message(mlv_normal, "invalid `node_id' [%s]\n", node1);
        return 0;
    }
    res = hexstr2data(n->pubkey, public_key_size, pub1);
    if(res != public_key_size) {
        message(mlv_normal, "invalid `pubkey' [%s]\n", pub1);
        return 0;
    }

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
