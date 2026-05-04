#ifndef _MS_KNOWN_DB_H
#define _MS_KNOWN_DB_H

#include "crypdf.h"

struct ms_known_node;
struct ms_known_node_db;

struct ms_known_node_db* make_kndb();
void dispose_kndb(struct ms_known_node_db* db);

void kndb_add_node(struct ms_known_node_db* db, 
                   struct ms_known_node* node);

struct ms_known_node* kndb_find_node(struct ms_known_node_db* db, 
                                     unsigned char id[node_id_size]);

struct ms_known_node_db* load_kndb(char* fname);

#endif
