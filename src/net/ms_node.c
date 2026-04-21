#include <stdlib.h>
#include <string.h>

#include "ms_node.h"
#include "../message.h"
#include "../fileutil.h"
#include "socks.h"

struct ms_node* make_node()
{
    struct ms_node* node;
    node = malloc(sizeof(*node));

    node->state = msns_init;

    node->sock = -1;
    node->the_cfg = NULL;
    node->id = NULL;
    ms_sess_coll_init(&node->peers);

    return node;
}

void set_node_cfg(struct ms_node*n, struct ms_node_cfg* cfg)
{
    n->the_cfg = cfg;
}


int load_node_id(struct ms_node*n)
{
    struct ms_nodeid_file* id;
    char* keyfile;
    int lfail;

    if(n->id) {
        dispose_nodeid(n->id);
    }
    id = make_nodeid();

    keyfile = concat_path(n->the_cfg->keys_dir, "id");
    lfail = load_nodeid_file(id, keyfile);
    free(keyfile);
    if(lfail) {
        dispose_nodeid(id);
        return 1;
    }
    n->id = id;
    return 0;
}

#if 0
int load_node_cfg(struct ms_node* n, const char* fname)
{
    n->the_cfg = make_node_cfg();
    return read_node_cfg_file(n->the_cfg, fname);
}
#endif

int kill_node(struct ms_node* n)
{
    
    return 0;
}

int start_node(struct ms_node *n)
{
    int sock;
    if(!n->the_cfg) {
        message_perror(mlv_normal, "ERROR", "node config not set");
        return mssn_no_config;
    }

    if(!n->id) {
        message_perror(mlv_normal, "ERROR", "node id not set");
        return mssn_no_id;
    }
    
    sock = make_sock(SOCK_DGRAM, n->the_cfg->listen_ip, n->the_cfg->listen_port);
    if(sock == -1) {
        return -1;
    }
    n->sock = sock;    
    n->state = msns_active;
    return 0;
}


