#ifndef _MS_CON_PARSER_H
#define _MS_CON_PARSER_H

#include <stddef.h>

struct ms_ctl_session;


enum ms_ccmd_header_len {
    ccmd_bind_hlen =  1,  /* 1 byte for port */
    ccmd_close_hlen = 1, /* 1 byte for code */

    ccmd_stat_hlen = 0,
    ccmd_send_hlen = 0,
    ccmd_recv_hlen = 0,
};

enum ms_ctlparser_state {
    cps_init = -1,
    cps_fin = 0,

    cps_reading_prefix,
    cps_reading_header,
    cps_reading_data
};

enum ms_ctlparser_res {
    cpres_conn_closed = -4,
    cpres_fatal = -3,
    cpres_error = -2,
    cpres_undef = -1,

    cpres_finished = 0,
    cpres_continue,
    cpres_want_more
};

enum {
    ms_conn_iport_undef = -1,
    ms_conn_iport_all = 0,
    ms_conn_iport_max = 16,
    parser_inner_buf_size = 4000
};


enum ccmd_type {
    ccmd_undef = -1,
    ccmd_bind = 1,
    ccmd_stat,
    ccmd_send,
    ccmd_recv,
    ccmd_close,
};

struct ms_ccmd_parsed {
    int type;
    int status;

    unsigned int data_len;
    
    union {
        struct {
            int iport;
        } bind;
        struct {
            unsigned char is_named;
            char peer_name[32];
            unsigned long ip;
            unsigned short port;
        } stat;
        struct {
            unsigned char is_named;
            char peer_name[32];
            unsigned long remote_ip;
            unsigned short remote_port;

            unsigned char *body;
            unsigned int body_len;
            unsigned int body_bytes_read;
        } send;
        struct {
            int code;
        } close;
    } u;
};

struct ms_cparser {
    int binary_mode;
    int state;
    unsigned int wanted_len;
    struct ms_ctl_session* the_session;

    unsigned char buf[parser_inner_buf_size];
    unsigned int buf_rd_pos;
    unsigned int buf_filled;

    struct ms_ccmd_parsed cur_cmd;
};

int ms_cparser_init(struct ms_cparser* cp, struct ms_ctl_session* ses);
void ms_cparser_reset(struct ms_cparser* cp);
int ms_ctlparser_read(struct ms_cparser* cp);
void ms_cparser_cleanup(struct ms_cparser* cp);

#endif /* _MS_COMM_PARSER_H */
