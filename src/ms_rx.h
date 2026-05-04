#ifndef _MS_RX_H
#define _MS_RX_H

#include <sue/sue_base.h>
#include "crypdf.h"

struct ms_peer_collection;
struct ms_transmit_queue;
struct ms_known_node_db;

struct ms_node_cfg;
struct ms_nodeid_file;

struct ms_udp_receiver {
    struct sue_fd_handler fdh;
    struct sue_timeout_handler tmoh;
    struct sue_event_selector *the_selector;

    struct ms_peer_collection* peers;
    struct ms_transmit_queue* txq;
    struct ms_known_node_db* kndb;

    struct ms_node_cfg* the_cfg;
    struct ms_nodeid_file* id;
};

struct ms_udp_receiver* make_udp_receiver(struct sue_event_selector* s, struct ms_node_cfg* cfg);
int start_udp_receiver(struct ms_udp_receiver* rx);
void dispose_udp_receiver(struct ms_udp_receiver* rx);

int do_we_know(struct ms_udp_receiver* rx, unsigned char id[node_id_size]);
void init_conn_to_known(struct ms_udp_receiver* rx, unsigned char id[node_id_size]);
int can_send_to_known(struct ms_udp_receiver* rx, unsigned char id[node_id_size]);
int send_to_known(struct ms_udp_receiver* rx, 
                  unsigned char id[node_id_size],
                  void* buf, int len);

int send_to(int fd, unsigned int ip, unsigned short port,
            const void *buf, int len);

#endif
