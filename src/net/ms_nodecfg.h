#ifndef _MS_NODECFG_H
#define _MS_NODECFG_H

struct ms_node_config {
};

void ms_nodecfg_free(struct ms_node_config* nc);

int node_load_cfg(struct ms_node_config*, const char* path);

#endif // !_MS_NODECFG_H

