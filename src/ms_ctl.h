#ifndef _MS_CONTROL_H
#define _MS_CONTROL_H

#include <stdio.h>
#include <sue/sue_base.h>

#include "ms_ctlpar.h"

struct sue_event_selector;
struct ms_node_cfg;
struct ms_peer;
struct ms_udp_receiver;

struct received_post {
    int src_iport, dst_iport;
    long long recv_time;

    char* payload;
    int payload_len;

    struct received_post* next;
};

struct ms_ctl_session {
    struct ms_ctl_session* next;

    struct ms_control_receiver *master;
    struct sue_fd_handler fdh;
    FILE *stream;

    struct extra_log* log;

    int id;

    int iport;
    char bound;

    char to_close;

    int rxq_len;
    struct received_post *rxq_first, *rxq_last;

    struct ms_cparser parser;
};


struct ms_control_receiver {
    struct sue_fd_handler fdh;
    struct sue_event_selector *the_selector;

    struct ms_udp_receiver* the_rx;
    struct ms_node_cfg* the_cfg;

    int ses_id_counter;
    struct ms_ctl_session* first;
    struct ms_ctl_session* ports[ms_ctl_iport_max];
    char *path;
};

enum ctl_errors {
    ctl_err_ok = 0,

    ctl_err_invalid_arg = 1,

    ctl_err_bind_port_occupied = 101,
    ctl_err_bind_already_bound,

    ctl_err_send_not_bound = 201,
    ctl_err_send_invalid_iport,
    ctl_err_send_msg_too_long,
    ctl_err_send_no_assoc,
};

extern int ms_ctl_errno;

void ctl_add_recvd(struct ms_ctl_session* ses, 
                   int src_iport, int dst_iport,
                   const unsigned char* payload, int len);

const char* ses_description(const struct ms_ctl_session* ses);

struct ms_control_receiver *
launch_control_receiver(struct sue_event_selector *sel,
                        struct ms_node_cfg *cfg,
                        struct ms_udp_receiver *rx);

void dispose_control_receiver(struct ms_control_receiver *crx);

void ctl_handle_shutdown(struct ms_ctl_session* ses);

/* bool, on error look ms_ctl_errno*/
int ctl_handle_bind(struct ms_ctl_session* ses, int iport);

/* bool, on error look ms_ctl_errno*/
int ctl_handle_send(struct ms_ctl_session* ses, 
                    struct ms_peer* peer,
                    int dst_iport,
                    const void* data, unsigned long long len);

void ctl_handle_stat(struct ms_ctl_session* ses, int iport);

int ctl_resolve_peer_from_name(struct ms_control_receiver* crx, char* name, 
                              unsigned int *ip, unsigned short* port);

#endif
