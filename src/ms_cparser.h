#ifndef _MS_COMM_PARSER_H
#define _MS_COMM_PARSER_H

#include "trie.h"
#include "crypdf.h"

enum ms_ccmd_type{
    ms_ccmd_undef = -1,
    ms_ccmd_bind,
    ms_ccmd_chmod,
    ms_ccmd_stat,
    ms_ccmd_send,
};

enum ms_cparser_mode{
    ms_cpm_undef,
    ms_cpm_text,
    ms_cpm_binary,
};

enum ms_cparser_state{
    ms_cps_init = 0,
    ms_cps_reading_header,
    ms_cps_disposing_header,
    ms_cps_reading_body,
    ms_cps_disposing_body,
    ms_cps_fin,
    ms_cps_fin_error,
    ms_cps_fin_fatal,
};


enum ms_cparser_res{
    ms_cp_res_unknown,
    ms_cp_res_finished,
    ms_cp_res_want_more,
    ms_cp_res_fatal,
    ms_cp_res_error,
};

enum {
    ms_conn_iport_undef = -1,
    ms_conn_iport_all = 0,

    ms_conn_iport_max = 16,

    parser_inner_buf_size = 4000,
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
    enum ms_ccmd_type type;
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
    } u;
};

struct ms_cparser {
    enum ms_cparser_state state;
    enum ms_cparser_mode mode;
    struct trie cmd_trie;
    struct ms_ccmd* target;
    
    unsigned char buf[parser_inner_buf_size];
    unsigned char* buf_p;
    unsigned int buf_used;
};

struct ms_ccmd* make_cmd();

/*clears cmd and inits it for new desired type*/
void cmd_init(struct ms_ccmd* cmd, int typy);
void dispose_cmd(struct ms_ccmd* cmd);

/* aside from state and mode initiation also fills search tree*/
int ms_cparser_init(struct ms_cparser* cp, struct ms_ccmd* target,
                    int start_mode);

/* before read next command */
void ms_cparser_reset(struct ms_cparser* cp);

void ms_cparser_switch_mode(struct ms_cparser* cp, int new_mode);

int ms_cparser_read(struct ms_cparser* cp, int fd);

/* frees tree*/
void ms_cparser_cleanup(struct ms_cparser* cp);

#endif
