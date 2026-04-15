#include <stdlib.h>

#include "ms_nodecfg.h"
#include "../fileutil.h"

struct ms_node_cfg* make_node_cfg()
{
    struct ms_node_cfg* n;
    n = malloc(sizeof(struct ms_node_cfg));
    
    n->listen_ip = 0;    
    n->listen_port = 0;    

    n->keys_dir = NULL;

    n->allow_trust_conn = 0;

    n->sessions_limit = 0;
    n->session_timeout = 0;

    return n;
}

void settle_keydir(struct ms_node_cfg* cfg)
{
    settle_localpath(&cfg->keys_dir, ".ms/keys");
}

struct ms_node_cfg* make_node_def_cfg()
{
    struct ms_node_cfg* n;
    n = make_node_cfg();
    
    n->listen_ip = mscfg_def_ip;
    n->listen_port = mscfg_def_port;

    n->allow_trust_conn = mscfg_def_truct_conn;

    n->session_timeout = mscfg_def_sess_timeout;
    n->sessions_limit = mscfg_def_sess_limit;

    settle_keydir(n);
    return n;
}

