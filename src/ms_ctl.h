#ifndef _MS_CONTROL_H
#define _MS_CONTROL_H

#include <stdio.h>
#include <sue/sue_base.h>

#include "ms_ctlpar.h"

struct sue_event_selector;
struct ms_node_cfg;
struct ms_peer;
struct ms_udp_receiver;

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

    struct ms_cparser parser;
};


struct ms_control_receiver {
    struct sue_fd_handler fdh;
    struct sue_event_selector *the_selector;

    struct ms_udp_receiver* the_rx;
    struct ms_node_cfg* the_cfg;

    int ses_id_counter;
    struct ms_ctl_session* first;
    struct ms_ctl_session* ports[ms_conn_iport_max];
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

int ctl_resolve_peer_from_str(char* str, 
                              unsigned int *ip, unsigned short* port);

#endif
