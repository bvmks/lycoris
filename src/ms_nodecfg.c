#include <stdlib.h>

#include "ms_nodecfg.h"
#include "fileutil.h"
#include "keyutils.h"


int peer_conf_has_ip(const struct peer_conf *pc)
{
    return pc->port != 0 && pc->ip != PEER_IP_UNDEF;
}

int peer_conf_has_id(const struct peer_conf *pc)
{
    return !all_zeroes(pc->node_id, node_id_size);
}

struct ms_node_cfg* make_node_cfg()
{
    struct ms_node_cfg* p;
    p = malloc(sizeof(struct ms_node_cfg));
    
    p->listen_ip = ntohl(mscfg_def_ip);
    p->listen_port = mscfg_def_port;    

    p->kndb_dir = NULL;
    p->keys_dir = NULL;
    
    p->cooldown_timeout = mscfg_def_cooldown_timeout;
    p->peer_timeout = mscfg_def_peer_timeout;
    p->keepalive_interval = 120;
    p->first_peer = NULL;

    return p;
}

void dispose_node_cfg(struct ms_node_cfg *p)
{
    if(p->keys_dir)
        free(p->keys_dir);
    if(p->kndb_dir)
        free(p->kndb_dir);
    while(p->first_peer) {
        struct peer_conf *tmp = p->first_peer;
        p->first_peer = p->first_peer->next;
        free(tmp);
    }
    free(p);
}

static void settle_keydir(struct ms_node_cfg* cfg)
{
    settle_localpath(&cfg->keys_dir, ".ms/keys");
}

static void settle_kndb_dir(struct ms_node_cfg* cfg)
{
    settle_localpath(&cfg->kndb_dir, ".ms/known.db");
}

struct ms_node_cfg* make_node_def_cfg()
{
    struct ms_node_cfg* n;
    n = make_node_cfg();

    settle_keydir(n);
    settle_kndb_dir(n);
    return n;
}

