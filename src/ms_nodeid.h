#ifndef _MS_NODECFG_H
#define _MS_NODECFG_H

#include "crypdf.h"
#include "ms_nodecfg.h"



struct ms_nodeid_file {
    unsigned char node_id[node_id_size];
    unsigned char secret[node_secret_size];
    unsigned char master_privat_key[secret_key_size];
    unsigned char master_public_key[public_key_size];

    unsigned char cookish[cookish_size];
};


void nodeid_init(struct ms_nodeid_file* ni);

struct ms_nodeid_file* make_nodeid();
void dispose_nodeid(struct ms_nodeid_file* ni);

struct ms_nodeid_file* load_node_id(struct ms_node_cfg *cfg);

int load_nodeid_file(struct ms_nodeid_file* ni, const char* fname);

#endif // !_MS_NODECFG_H

