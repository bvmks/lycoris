#ifndef _MS_NODE_CFG_H
#define _MS_NODE_CFG_H

#include <netinet/in.h>

enum {
    mscfg_def_ip = INADDR_ANY,
    mscfg_def_port = 24880,

    mscfg_def_truct_conn = 1,

    mscfg_def_sess_limit = 5,
    mscfg_def_sess_timeout = 10000,

};


struct ms_node_cfg {
    unsigned int listen_ip;
    unsigned short listen_port;

    char* keys_dir;

    int allow_trust_conn;

    int sessions_limit;
    int session_timeout;
};

struct ms_node_cfg* make_node_cfg();

struct ms_node_cfg* make_node_def_cfg();

int read_node_cfg_file(struct ms_node_cfg* cfg, const char* fname);

void dispose_node_cfg(struct ms_node_cfg* cfg);

void settle_keydir(struct ms_node_cfg* cfg);
#endif
