#ifndef _MS_COMM_PARSER_H
#define _MS_COMM_PARSER_H

#include "trie.h"

enum ms_ccom_type{
    ms_ccmd_bind,
    ms_ccmd_stat,
};

enum ms_cparser_mode{
    ms_cpm_text,
    ms_cpm_binary,
};

enum ms_cparser_state{
    ms_cps_init,
    ms_cps_reading_header,
    ms_cps_disposing_header,
    ms_cps_reading_body,
    ms_cps_disposing_body,
    ms_cps_fin,
    ms_cps_fin_error,
    ms_cps_fin_fatal,
};


enum ms_cparser_res{
    ms_cp_res_want_more,
    ms_cp_res_finished,
    ms_cp_res_fatal,
    ms_cp_res_error,
};

struct ms_ccmd {
    enum ms_ccom_type type;
    unsigned int iport;
    unsigned char *body_buf;
    unsigned long long body_len;
    unsigned long long body_bytes_read;
};

struct ms_cparser {
    enum ms_cparser_state state;
    enum ms_cparser_mode mode;
    struct trie cmd_trie;
    
    unsigned char line_buf[1024];
    unsigned long long buf_used;
    
    unsigned long long direct_wanted_bytes;
    void *direct_ptr;
};

struct ms_ccmd* make_cmd();
void cmd_reset(struct ms_ccmd* cmd);
void dispose_cmd(struct ms_ccmd* cmd);

/* creates and fills search tree*/
int ms_cparser_init(struct ms_cparser* cp, int start_mode);

/* before read next command */
void ms_cparser_reset(struct ms_cparser* cp);

void ms_cparser_switch_mode(struct ms_cparser* cp, int new_mode);

/*  if parser want direct write will return ptr to buf in ms_ccommand 
    otherwise will return NULL*/
void* ms_cparser_get_direct(struct ms_cparser *cp);

/*  if parser want direct write will return num of bytes needed
    otherwise will return 0*/
unsigned long long ms_cparser_want_direct(struct ms_cparser *cp);

int ms_cparser_feed(struct ms_cparser* cp, struct ms_ccmd* cmd,
                    unsigned char* data,
                    unsigned long long datalen, long long* read,
                    long long int direct);

/* frees inner allocated bufer if it is*/
void ms_cparser_cleanup(struct ms_cparser* cp);

#endif
