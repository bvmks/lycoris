#ifndef _MS_NODE_H
#define _MS_NODE_H

#include "crypdf.h"
#include "ms_nodecfg.h"

struct ms_node_id {
    unsigned char secret[node_secret_size];
    unsigned char public_key[public_key_size];
    unsigned char node_id[node_id_size];
};

struct ms_node {
    struct ms_node_id identity;
    struct ms_node_config cfg;
};

struct ms_node* make_node();

int ms_node_load(struct ms_node* n, const char* cfg_path);

#endif
