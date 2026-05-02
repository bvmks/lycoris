#ifndef _MS_RX_H
#define _MS_RX_H

#include <sue/sue_base.h>
#include "ms_nodecfg.h"

struct ms_udp_receiver {
    struct sue_fd_handler fd_h;
    struct sue_timeout_handler tmo_h;
    struct sue_event_selector *the_selector;
    struct ms_peer_collection* peers;
    struct ms_transmit_queue* txq;

    struct ms_node_cfg* the_cfg;
    struct ms_nodeid_file* id;
};


struct ms_udp_receiver* make_udp_receiver(struct sue_event_selector* s, struct ms_node_cfg* cfg);

int start_udp_receiver(struct ms_udp_receiver* n);

void dispose_udp_receiver(struct ms_udp_receiver* n);

int send_to(int fd, unsigned int ip, unsigned short port,
            const void *buf, int len);

#endif
