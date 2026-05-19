#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ms_cparser.h"
#include "utils.h"
#include "log.h"


typedef int (*ccmd_cb)(struct ms_cparser*, const char*, int);

struct ms_ccmd* make_cmd()
{
    struct ms_ccmd* res = malloc(sizeof(*res));
    memset(res, 0, sizeof(*res));
    return res;
}

int handle_bind_header(struct ms_cparser* cp, const char* buf, int len) 
{

}


int handle_chmod_header(struct ms_cparser* cp, const char* buf, int len) 
{

}

int handle_stat_header(struct ms_cparser* cp, const char* buf, int len) 
{

}

int handle_send_header(struct ms_cparser* cp, const char* buf, int len) 
{

}

int ms_cparser_init(struct ms_cparser* cp, struct ms_ccmd* target,
                    int start_mode)
{
    memset(cp, 0, sizeof(*cp));
    trie_init(&cp->cmd_trie);

    void** slot;
    
    slot = trie_provide(&cp->cmd_trie, "BIND");
    *slot = &handle_bind_header;

    slot = trie_provide(&cp->cmd_trie, "CHMOD");
    *slot = &handle_chmod_header;
    
    slot = trie_provide(&cp->cmd_trie, "STAT");
    *slot = &handle_stat_header;

    slot = trie_provide(&cp->cmd_trie, "SEND");
    *slot = &handle_send_header;
    
    cp->target = target;
    cp->mode = start_mode;
    cp->buf_p = cp->buf;
    ms_cparser_reset(cp);
    return 1;
}


static void cleanup_cmd(struct ms_ccmd* cmd)
{
    switch (cmd->type) {
    case ms_ccmd_undef: 
        break;
    case ms_ccmd_bind: 
        cmd->u.bind.iport = ms_conn_iport_undef;
        break;
    case ms_ccmd_stat: 
        break;
    case ms_ccmd_send: 
        if(cmd->u.send.body)
            free(cmd->u.send.body);
        break;
    default:
        log_msg(llv_alert, "unexpected value for control command type (%d) (BUG)", cmd->type);
        break;
    }
}


void cmd_init(struct ms_ccmd* cmd, int type)
{
    cleanup_cmd(cmd);
    memset(cmd, 0, sizeof(*cmd));
    cmd->type = type;
    switch (type) {
    case ms_ccmd_undef: 
        break;
    case ms_ccmd_bind: 
        cmd->u.bind.iport = ms_conn_iport_undef;
        break;
    case ms_ccmd_chmod: 
        cmd->u.chmod.new_mode = ms_cpm_undef;
        break;
    case ms_ccmd_stat: 
        cmd->u.stat.addr.addr_resolve_type = stat_resolve_undef;
        break;
    case ms_ccmd_send: 
        cmd->u.send.addr.addr_resolve_type = stat_resolve_undef;
        break;
    default:
        log_msg(llv_alert, "unexpected value for comand type (%d) (BUG)", cmd->type);
        break;
    }
}

void dispose_cmd(struct ms_ccmd* cmd)
{
    cleanup_cmd(cmd);
    free(cmd);
}

void ms_cparser_reset(struct ms_cparser* cp)
{
    cp->state = ms_cps_init;
    cp->buf_used = 0;
    if(cp->target)
        cmd_init(cp->target, ms_ccmd_undef);
}

void ms_cparser_switch_mode(struct ms_cparser* cp, int new_mode)
{
    if (cp) cp->mode = new_mode;
}

void ms_cparser_cleanup(struct ms_cparser* cp)
{
    trie_clear(&cp->cmd_trie);
}


static int parse_binary(struct ms_cparser* cp, 
                        long long* readed)
{
    log_msg(llv_alert, "parse_binary not supported");
    *readed = 0;
    return ms_cp_res_fatal;
}

static const char* ms_cps2a(int state)
{
    switch (state) {
    case ms_cps_init: return "init";
    case ms_cps_reading_header: return "reading_header";
    case ms_cps_disposing_header: return "disposing_header";
    case ms_cps_reading_body: return "reading_body";
    case ms_cps_disposing_body: return "disposing_body";
    case ms_cps_fin: return "fin";
    case ms_cps_fin_error: return "fin_error";
    case ms_cps_fin_fatal: return "fin_fatal";
    }
    return "unknown";
}

static int cps2cp_res(int state) {
    switch (state) {
    case ms_cps_init: 
    case ms_cps_reading_header:
    case ms_cps_disposing_header:
    case ms_cps_reading_body:
    case ms_cps_disposing_body:
        return ms_cp_res_want_more;

    case ms_cps_fin:
    case ms_cps_fin_fatal:
        return ms_cp_res_finished;

    case ms_cps_fin_error: 
        return ms_cp_res_error;
    default: 
        return ms_cp_res_unknown;
    }
}

static int read_header(struct ms_cparser* cp, long long* readed, int mode) {
    unsigned int wanted = 0;
    unsigned int rd = 0;
    ccmd_cb cb;
    while(cp->buf_p < cp->buf_used) 
    {
        if(cp->buf[cp->buf_p] == '\n') {
            if(cp->buf_used > 0 &&
               cp->buf[cp->buf_used - 1] == '\n') 
            {
                /*end of the header body*/
            }
        }
        cp->buf_p++;
        rd++;
    }
    *readed = rd;
    return ms_cp_res_want_more;
}

static int dispose_header(struct ms_cparser* cp, long long* readed, int mode) {
}

static int inner_parse(struct ms_cparser* cp, 
                       long long* readed,
                       int mode)
{
    switch (cp->state) {
    case ms_cps_init:
        cp->state = ms_cps_reading_header;
    case ms_cps_reading_header:
        read_header(cp, readed, mode);
        break;
    case ms_cps_disposing_header:
        dispose_header(cp, readed, mode);
        break;
    case ms_cps_reading_body:
    case ms_cps_disposing_body:
        break;
        
    case ms_cps_fin:
    case ms_cps_fin_error:
    case ms_cps_fin_fatal:
        log_msg(llv_alert, "ms_cparser_read called with %s state (BUG)", ms_cps2a(cp->state));
        return cp->state;
    default:
        log_msg(llv_alert, "unexpected value for parser state (%d) (BUG)", cp->state);
        return ms_cp_res_fatal;
    }


}

int ms_cparser_read(struct ms_cparser* cp, int fd)
{
    long long rd;
    long long parser_readed;
    int parser_res = ms_cp_res_fatal;

    if (cp->buf_used < sizeof(cp->buf)) {
        rd = read(fd, cp->buf + cp->buf_used, sizeof(cp->buf) - cp->buf_used);
        if (rd <= 0) {
            if(rd == -1)
                log_perror(llv_alert, "ms_cparser_read", "read");
            return ms_cp_res_fatal; 
        }
        cp->buf_used += rd;
    }

    /* switch only to ensure that mode is valid */
    parser_readed = 0;
    switch (cp->mode) {
    case ms_cpm_text:
    case ms_cpm_binary:
        parser_res = inner_parse(cp, &parser_readed, cp->mode);
    break;
    default: 
        log_msg(llv_alert, "unexpected value for parser mode (%d) (BUG)", cp->mode);
        parser_res = ms_cp_res_fatal;
    }

    if(parser_readed > 0) {
        memmove(cp->buf, cp->buf + parser_readed, cp->buf_used - parser_readed);
        cp->buf_used -= parser_readed;
        cp->buf_p -= parser_readed;
    }
    return parser_res;
}
