#ifndef _MS_CONTROL_H
#define _MS_CONTROL_H

#include <stdio.h>
#include <sue/sue_base.h>

#include "ms_cparser.h"

struct sue_event_selector;
struct ms_node_cfg;
struct ms_udp_receiver;

struct ms_con_session {
    struct ms_con_session* next;

    struct ms_control_receiver *the_master;
    struct sue_fd_handler fdh;
    FILE *stream;

    struct extra_log* log;

    int id;

    int iport;
    char bound;

    char to_close;

    struct ms_ccmd* cur_cmd;
    struct ms_cparser parser;
};


struct ms_control_receiver {
    struct sue_fd_handler fdh;
    struct sue_event_selector *the_selector;

    struct ms_udp_receiver* the_rx;
    struct ms_node_cfg* the_cfg;

    int ses_id_counter;
    struct ms_con_session* first;
    struct ms_con_session* ports[ms_conn_iport_max];
    char *path;
};


struct ms_control_receiver *
launch_control_receiver(struct sue_event_selector *sel,
                           struct ms_node_cfg *cfg,
                           struct ms_udp_receiver *rx);

void dispose_control_receiver(struct ms_control_receiver *crx);

#endif
