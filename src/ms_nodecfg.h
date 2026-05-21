#ifndef _MS_NODE_CFG_H
#define _MS_NODE_CFG_H

#include <netinet/in.h>
#include "crypdf.h"

enum {
    mscfg_def_ip = INADDR_ANY,
    mscfg_def_port = 24880,

    mscfg_def_cooldown_timeout = 360,
    mscfg_def_peer_timeout = 60,
    mscfg_def_keepalive_interval = 10,
};

enum {peer_name_length_limit = 60};

enum ms_peer_type {
    ptp_undef = -1,
    ptp_peer,
    ptp_server,
    ptp_mynode,
};

struct peer_conf {
    char name[peer_name_length_limit + 1];
    int type;
    unsigned int ip;
    unsigned short port;
    unsigned char node_id[node_id_size];
    struct peer_conf *next;
};

#define PEER_IP_UNDEF ((unsigned int)(-1))

int peer_conf_has_ip(const struct peer_conf *pc);
int peer_conf_has_id(const struct peer_conf *pc);

struct ms_node_cfg {
    unsigned int listen_ip;
    unsigned short listen_port;

    char* kndb_dir;
    char* keys_dir;
    char* peers_cfg_file;

    int has_control_sock;
    char* control_sock_path;

    int cooldown_timeout, peer_timeout, keepalive_interval;
    struct peer_conf *first_peer;
};



struct ms_node_cfg* make_node_cfg();

int read_node_cfg_file(struct ms_node_cfg* cfg, const char* fname);

void dispose_node_cfg(struct ms_node_cfg* cfg);


void settle_keys_dir(struct ms_node_cfg* cfg);

void settle_kndb_dir(struct ms_node_cfg* cfg);

void settle_ctlsock_path(struct ms_node_cfg* cfg);

void settle_peerscfg_path(struct ms_node_cfg* cfg);

#endif
