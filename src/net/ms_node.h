#ifndef _MS_NODE_H
#define _MS_NODE_H

#include "crypdf.h"

struct ms_node_config {
    unsigned char node_id[node_id_size];
    unsigned char secret[node_secret_size];
    unsigned char public_key[public_key_size];
};

struct ms_node {
    struct ms_node_config cfg;
};

#endif
