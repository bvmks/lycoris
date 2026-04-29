#ifndef _MS_NODE_H
#define _MS_NODE_H

#include "ms_nodeid.h"
#include "ms_nodecfg.h"
#include "ms_peer.h"
#include "../events.h"

/* start_node error codes */
enum {
    mssn_no_config = -1,
    mssn_no_id = -2,
};

enum ms_node_state {
    msns_init,
    msns_active,
    msns_terminated,
};

struct ms_node {
    enum ms_node_state state;

    unsigned char cookish[cookish_size];

    struct ms_nodeid_file* id;
    struct ms_peer_collection* peers;
    struct ms_node_cfg* the_cfg;
};

struct ms_node* make_node();
void set_node_cfg(struct ms_node*n, struct ms_node_cfg* cfg);
int load_node_id(struct ms_node*n);
int start_node(struct ms_node* n);
int kill_node(struct ms_node* n);

void dispose_node(struct ms_node* n);

#endif
