#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ms_chandl.h"
#include "ms_con.h"
#include "ms_cparser.h"
#include "utils.h"
#include "log.h"


static const char* ms_cps2a(int state)
{
    switch (state) {
    case ms_cps_init:               return "init";
    case ms_cps_reading_header:     return "reading_header";
    case ms_cps_reading_body:       return "reading_body";
    case ms_cps_disposing_body:     return "disposing_body";
    case ms_cps_fin:                return "fin";
    case ms_cps_fin_error:          return "fin_error";
    case ms_cps_fin_fatal:          return "fin_fatal";
    }
    return "unknown";
}

static int txt_parse_bind_ccmd(struct ms_cparser* cp, const char* args) {
    struct ms_con_session* ses = cp->the_session;
    int bind_iport = ms_conn_iport_undef;

    if(sscanf(args, " %d ", &bind_iport) < 1) {
        log_msg(llv_debug, 
                "CONTROL SESSION [%d][%s]: BIND command doesn't have a valid argument",
                ses->id, 
                ses->bound ? decimal2a(ses->iport) : "-");
        send_code(ses, ms_ccmd_rc_parse_error);
        return ms_cp_res_error;
    }
    cp->target->type = ms_ccmd_bind;
    cp->target->u.bind.iport = bind_iport;
    return ms_cp_res_finished;
}

static int txt_parse_chmod_ccmd(struct ms_cparser* cp, const char* args) {
    struct ms_con_session* ses = cp->the_session;
    struct ms_ccmd* tar = cp->target;
    char mode_str[16];

    if(sscanf(args, " %15s ", mode_str) < 1) {
        log_msg(llv_debug, 
                "CONTROL SESSION [%d][%s]: CHMOD command doesn't have a valid argument",
                ses->id, 
                ses->bound ? decimal2a(ses->iport) : "-");
        send_code(ses, ms_ccmd_rc_parse_error);
        return ms_cp_res_error;
    }
    tar->type = ms_ccmd_chmod;
    memcpy(tar->u.chmod.new_mode_str, mode_str, mode_str_max_len);
    tar->u.chmod.new_mode_str[mode_str_max_len] = 0;
    return ms_cp_res_finished;
}

static int txt_parse_close_ccmd(struct ms_cparser* cp, const char* args) {
    struct ms_ccmd* tar = cp->target;

    tar->type = ms_ccmd_close;
    tar->u.close.code = 0;
    return ms_cp_res_finished;
}

typedef int (*ccmd_cb)(struct ms_cparser*, const char*);

struct txt_ccmd {
    int ccmd;
    char* name;
    ccmd_cb cb;
};

static struct txt_ccmd txt_ccmds[] = {
    {ms_ccmd_bind,  "BIND",  &txt_parse_bind_ccmd},
    {ms_ccmd_chmod, "CHMOD", &txt_parse_chmod_ccmd},
    {ms_ccmd_stat,  "STAT",  NULL},
    {ms_ccmd_send,  "SEND",  NULL},
    {ms_ccmd_close, "CLOSE", &txt_parse_close_ccmd},
};

static struct trie cmd_trie;

struct ms_ccmd* make_cmd()
{
    struct ms_ccmd* res = malloc(sizeof(*res));
    memset(res, 0, sizeof(*res));
    return res;
}

void fill_cmd_trie()
{
    int i, total_cmds;
    void** slot;
    static int t = 1;

    if(!t)
        return;

    trie_init(&cmd_trie);

    total_cmds = sizeof(txt_ccmds) / sizeof(struct txt_ccmd);
    for(i = 0; i < total_cmds; i++) {
        slot = trie_provide(&cmd_trie, txt_ccmds[i].name);
        *slot = txt_ccmds[i].cb;
    }
    t = 0;
}

void clear_cmd_trie() {
    trie_clear(&cmd_trie);
}

int ms_cparser_init(struct ms_cparser* cp, struct ms_ccmd* target,
                    struct ms_con_session* ses,
                    int start_mode)
{
    memset(cp, 0, sizeof(*cp));
    cp->target = target;
    cp->mode = start_mode;
    cp->the_session = ses;
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
    case ms_ccmd_chmod: 
        cmd->u.chmod.new_mode_str[0] = 0;
        break;
    case ms_ccmd_stat: 
        cmd->u.stat.addr.addr_resolve_type = stat_resolve_undef;
        break;
    case ms_ccmd_send: 
        cmd->u.send.addr.addr_resolve_type = stat_resolve_undef;
        if(cmd->u.send.body)
            free(cmd->u.send.body);
        cmd->u.send.body = NULL;
        cmd->u.send.body_bytes_read = 0;
        cmd->u.send.body_len = 0;
        break;
    case ms_ccmd_close: 
        cmd->u.close.code = 0;
        break;
    default:
        log_msg(llv_alert, " cleanup_cmd: unexpected value for control command type (%d) (BUG)", cmd->type);
        break;
    }
}


void cmd_init(struct ms_ccmd* cmd, int type)
{
    memset(cmd, 0, sizeof(*cmd));
    cmd->type = type;
    cleanup_cmd(cmd);
}

void dispose_cmd(struct ms_ccmd* cmd)
{
    cleanup_cmd(cmd);
    free(cmd);
}

void ms_cparser_reset(struct ms_cparser* cp)
{
    cp->state = ms_cps_init;
    if(cp->target)
        cmd_init(cp->target, ms_ccmd_undef);
}

static const char* mode2a (enum ms_cparser_mode mode) {
    switch (mode) {
    case ms_cpm_undef:  return "UNDEF";
    case ms_cpm_text:   return "TEXT";
    case ms_cpm_binary: return "BINARY";
    }
    return "UNKNOWN";
}

void ms_cparser_switch_mode(struct ms_cparser* cp, int new_mode)
{
    struct ms_con_session* ses = cp->the_session;
    log_msg(llv_debug, 
            "CONTROL SESSION [%d][%s]: switching mode to %s",
            ses->id, 
            ses->bound ? decimal2a(ses->iport) : "-",
            mode2a(new_mode));
    cp->mode = new_mode;
}

void ms_cparser_cleanup(struct ms_cparser* cp)
{
}



static int txt_parse_header(struct ms_cparser* cp) {
    struct ms_con_session* ses = cp->the_session;
    ccmd_cb cb;
    int cmd_len = 0;
    int r;
    
    int len = cp->buf_p - 2; 
    
    if(len <= 0) {
        log_msg(llv_alert, "empty control command header");
        cp->state = ms_cps_fin_error;
        return ms_cp_res_error;
    }
    
    cp->buf[len] = 0;

    cb = trie_get_with_len(&cmd_trie, (char*)cp->buf, &cmd_len);
    if(!cb) {
        log_msg(llv_debug, 
                "CONTROL SESSION [%d][%s]: unknown control command \"%s\"", 
                ses->id, 
                ses->bound ? decimal2a(ses->iport) : "-",
                cp->buf);
        send_code(ses, ms_ccmd_rc_parse_error);
        cp->state = ms_cps_fin_error;
        return ms_cp_res_error;
    }
    r = (*cb)(cp, (char*)cp->buf + cmd_len);
    switch (r) {
    case ms_cp_res_fatal:
        cp->state = ms_cps_fin_fatal;
        return r;
    case ms_cp_res_error:
        cp->state = ms_cps_fin_error;
        return r;
    case ms_cp_res_finished:
        cp->state = ms_cps_fin;
        return r;
    default:
        return r;
    }
}

static int find_block(struct ms_cparser* cp, long long* size) {
    while(cp->buf_p < cp->buf_used) 
    {
        if(cp->buf[cp->buf_p] == '\n' &&
           cp->buf_p > 0 &&
           cp->buf[cp->buf_p - 1] == '\n') 
        {
            cp->buf_p++;
            *size = cp->buf_p;
            return 1;
        }
        cp->buf_p++;
    }
    *size = 0;
    return 0;
}

static int inner_parse_txt(struct ms_cparser* cp,
                           long long* readed)
{
    long long rd;

    switch (cp->state) {
    case ms_cps_fin:
    case ms_cps_fin_error:
    case ms_cps_fin_fatal:
        log_msg(llv_alert, "ms_cparser_read called with %s state (BUG)", ms_cps2a(cp->state));
        return cp->state;

    case ms_cps_init:
        cp->state = ms_cps_reading_header;
    case ms_cps_reading_header:
    case ms_cps_reading_body:
    case ms_cps_disposing_body:
            break;
    }

    if(find_block(cp, &rd)) {
        *readed = rd;
        return txt_parse_header(cp);
    }
    *readed = 0;
    return ms_cp_res_want_more;
}

static int inner_parse(struct ms_cparser* cp,
                       long long* readed,
                       int mode)
{
    switch (mode) {
    case ms_cpm_text:
        return inner_parse_txt(cp, readed);
    case ms_cpm_binary:
        log_msg(llv_alert, "parse_binary not supported");
        *readed = 0;
        cp->state = ms_cps_fin_fatal;
        return ms_cp_res_fatal;
    }
    cp->state = ms_cps_fin_fatal;
    return ms_cp_res_fatal;
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
            return ms_cp_res_conn_closed; 
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
