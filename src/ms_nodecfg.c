#include <stdlib.h>

#include "stupid_peers_parser.h"
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

    p->keys_dir = NULL;
    p->kndb_dir = NULL;
    p->peers_cfg_file = NULL;

    p->has_control_sock = 0;
    p->control_sock_path = NULL;
    
    p->cooldown_timeout = mscfg_def_cooldown_timeout;
    p->peer_timeout = mscfg_def_peer_timeout;
    p->keepalive_interval = mscfg_def_keepalive_interval;
    p->first_peer = NULL;

    return p;
}

void dispose_node_cfg(struct ms_node_cfg *p)
{
    if(p->keys_dir)
        free(p->keys_dir);
    while(p->first_peer) {
        struct peer_conf *tmp = p->first_peer;
        p->first_peer = p->first_peer->next;
        free(tmp);
    }
    free(p);
}

void settle_keys_dir(struct ms_node_cfg* cfg)
{
    settle_localpath(&cfg->keys_dir, ".ms/keys");
}

void settle_kndb_dir(struct ms_node_cfg* cfg)
{
    settle_localpath(&cfg->kndb_dir, ".ms/kndb");
}

void settle_ctlsock_path(struct ms_node_cfg* cfg)
{
    settle_localpath(&cfg->control_sock_path, ".ms/ctlsock");
}

void settle_peerscfg_path(struct ms_node_cfg* cfg)
{
    settle_localpath(&cfg->peers_cfg_file, ".ms/peers.conf");
}

int read_node_cfg_file(struct ms_node_cfg* cfg, const char* fname)
{
    return 1;
}
