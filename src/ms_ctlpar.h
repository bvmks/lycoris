#ifndef _MS_COMM_PARSER_H
#define _MS_COMM_PARSER_H

#include "crypdf.h"

struct ms_ctl_session;

enum ms_ccmd_type{
    ccmd_undef = -1,
    ccmd_bind ,
    ccmd_chmod,
    ccmd_stat,
    ccmd_send,
    ccmd_close,
};

enum ms_ctlparser_mode{
    ctlparser_m_undef = -1,
    ctlparser_m_text,
    ctlparser_m_binary,
};

enum ms_ctlparser_state{

    ctlparser_s_init = -1,
    ctlparser_s_fin  = 0,

    ctlparser_s_reading_header,
    ctlparser_s_reading_data,
};


enum ms_ctlparser_res{
    ctlparser_res_conn_closed = -4,
    ctlparser_res_fatal = -3,
    ctlparser_res_error = -2,
    ctlparser_res_undef = -1,

    ctlparser_res_finished = 0,

    ctlparser_res_want_more,
    ctlparser_res_want_to_dispose,
};

enum {
    ms_conn_iport_undef = -1,
    ms_conn_iport_all = 0,

    ms_conn_iport_max = 16,

    parser_inner_buf_size = 4000,
    parser_max_line_len = 50,
    
    mode_str_max_len = 15,
};


union resolved_addr{
    enum {
        stat_resolve_undef = -1,
        stat_resolve_ipport,
        stat_resolve_name,  /* by configured peer name*/
        stat_resolve_dns,
    } addr_resolve_type;

    unsigned int ip;
    unsigned short port;
    char name[node_name_max_size + 1];
};

struct ms_ccmd {
    int type;
    int parse_failed;
    union {
        struct {
            int iport;
        } bind;

        struct {
            union resolved_addr addr;
        } stat;

        struct {
            int new_mode;
        } chmod;

        struct {
            union resolved_addr addr;
            long long msg_len;

            unsigned char *body;
            unsigned long long body_len;
            unsigned long long body_bytes_read;
        } send;

        struct {
            int code;
        } close;
    } u;
};

struct ms_ccmd_result {
    enum ms_ccmd_type type;
    union {

    }u;

    int code;
};

struct ms_cparser {
    int state;
    int mode;
    struct ms_ccmd* target;
    struct ms_ctl_session* the_session;

    char* txt_the_cur_line;
    
    unsigned char buf[parser_inner_buf_size];
    unsigned int buf_p;
    unsigned int buf_used;
};

void fill_cmd_trie();
void clear_cmd_trie();

struct ms_ccmd* make_cmd();

void cmd_init(struct ms_ccmd* cmd);
void dispose_cmd(struct ms_ccmd* cmd);

/* aside from state and mode initiation also fills search tree*/
int ms_cparser_init(struct ms_cparser* cp, struct ms_ccmd* target,
                    struct ms_ctl_session* ses,
                    int start_mode);

/* before read next command */
void ms_cparser_reset(struct ms_cparser* cp);

void ms_cparser_switch_mode(struct ms_cparser* cp, int new_mode);

int ms_ctlparser_read(struct ms_cparser* cp, int fd);

/* frees tree*/
void ms_cparser_cleanup(struct ms_cparser* cp);

#endif
