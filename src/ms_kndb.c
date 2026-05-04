#include "ms_kndb.h"

struct ms_known_node {
    char* name;
    unsigned char id[node_id_size];
    unsigned char pubkey[public_key_size];
};

struct ms_known_node_db {
    struct ms_known_node *first, *last;
};


struct ms_known_node_db* load_kndb(char* fname)
{
    /*TODO*/
    return 0;
}
