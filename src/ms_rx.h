#ifndef _MS_RX_H
#define _MS_RX_H

#include "../lib/sue/sue_base.h"
#include "ms_nodecfg.h"

struct ms_node {
    struct sue_fd_handler fd_h;
    struct sue_timeout_handler tmo_h;
    struct sue_event_selector *the_selector;
    struct ms_node_cfg* the_cfg;
    struct ms_nodeid_file* id;
    struct ms_peer_collection* peers;
    struct ms_transmit_queue* txq;
};


struct ms_node* make_node(struct sue_event_selector* s, struct ms_node_cfg* cfg);

int start_node(struct ms_node* n);
int kill_node(struct ms_node* n);

void dispose_node(struct ms_node* n);

int send_to(int fd, unsigned int ip, unsigned short port,
             const void *buf, int len);

int do_recv(struct ms_node*);

#endif
