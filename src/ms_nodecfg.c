#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stupid_peers_parser.h"
#include "ms_nodecfg.h"
#include "fileutil.h"
#include "keyutils.h"
#include "hexdata.h"
#include "addrport.h"
#include "rep.h"


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

const char *peer_type_str(int ptp)
{
    switch(ptp) {
    case ptp_default:    return "default";
    case ptp_mynode:     return "mynode";
    default:             return NULL;
    }
}

static const char *strprot(const char *s)
{
    return s ? s : "(unset)";
}

static const char *node_id_string(const unsigned char ni[])
{
    static char s[node_id_size * 2 + 1];
    if(all_zeroes(ni, node_id_size))
        return "unknown";
    hexdata2str(s, ni, node_id_size);
    return s;
}

static void
do_dump_configuration(report_callback f, void *ud, struct ms_node_cfg *cfg)
{
    struct peer_conf *p;
    int port = cfg->listen_port;

    f(ud, "listen_address/port %s",
          ipport2a(cfg->listen_ip, port));
    f(ud, "cooldown/peer timeouts %d/%d",
          cfg->cooldown_timeout, cfg->peer_timeout);
    f(ud, "keys_dir %s", strprot(cfg->keys_dir)); 

    f(ud, "control socket is %sabled", cfg->has_control_sock ? "en" : "dis"); 
    if(cfg->has_control_sock)
        f(ud, "control socket path: %s", strprot(cfg->control_sock_path)); 

    for(p = cfg->first_peer; p; p = p->next) {
        f(ud, "peer %s: type (%s) addr %s id %s",
              p->name, peer_type_str(p->type), ipport2a(p->ip, p->port),
              node_id_string(p->node_id));
    }
}


void dump_configuration_to_log(struct ms_node_cfg *cfg, int level)
{
    do_dump_configuration(report_to_log_cb, &level, cfg);
}

void dump_configuration_to_stream(struct ms_node_cfg *cfg, FILE *stream)
{
    do_dump_configuration(report_to_stream_cb, stream, cfg);
}
