#include <stdlib.h>

#include "ms_node.h"


struct ms_node* make_node()
{
    return malloc(sizeof(struct ms_node));
}

int ms_node_load(struct ms_node* n, const char* cfg_path)
{
   node_load_cfg(&n->cfg, cfg_path);
}
