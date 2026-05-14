#ifndef _MS_KNOWN_DB_H
#define _MS_KNOWN_DB_H

#include "crypdf.h"

enum {
    kndb_res_success           =  0,
    kndb_res_node_unknown      = -2,
};

struct ms_known_node;
struct known_nodes_db;
struct ms_node_cfg;

struct known_nodes_db* make_kndb(struct ms_node_cfg* cfg);
void dispose_kndb(struct known_nodes_db* db);

int kndb_get_node(struct known_nodes_db* db,
                  const unsigned char *node_id,
                  unsigned char *pubkey);

int test_init_kndb(struct known_nodes_db* db);

struct known_nodes_db* load_kndb(struct ms_node_cfg* cfg);

#endif
