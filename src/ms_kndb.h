#ifndef _MS_KNOWN_DB_H
#define _MS_KNOWN_DB_H

#include "crypdf.h"

enum {
    kndb_res_success           =  0,
    kndb_res_node_unknown      = -2,
    kndb_res_file_error        = -3,
    kndb_res_unexpected        = -4,
};


struct knnode_paths {
    char* node_dir;
    char* node_file;
};

struct ms_known_node {
    unsigned char id[node_id_size];
    unsigned char pubkey[public_key_size];
};

struct known_nodes_db {
    char* dir;
};

struct ms_node_cfg;

struct known_nodes_db* make_kndb(char* dir);

void dispose_kndb(struct known_nodes_db* db);

int kndb_get_node(struct known_nodes_db* db,
                  const unsigned char *node_id,
                  unsigned char *pubkey);

int kndb_save_node(struct known_nodes_db* db,
                   const unsigned char *node_id,
                   unsigned char *pubkey);

struct known_nodes_db* load_kndb(struct ms_node_cfg* cfg);

#endif
