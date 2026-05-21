#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "trie.h"
#include "ms_ctl.h"
#include "log.h"


static const char* parser_state2a(int state)
{
    switch (state) {
    case ctlparser_s_init:               return "init";
    case ctlparser_s_reading_block:      return "reading_block";
    case ctlparser_s_disposing_block:    return "disposing_block";
    case ctlparser_s_fin:                return "fin";
    case ctlparser_s_fin_error:          return "fin_error";
    case ctlparser_s_fin_fatal:          return "fin_fatal";
    }
    return "unknown";
}

static int parser_res2state(int res) {
    switch (res) {
    case ctlparser_res_conn_closed: return ctlparser_s_fin;
    case ctlparser_res_fatal: return ctlparser_s_fin_fatal;
    case ctlparser_res_error: return ctlparser_s_fin_error;

    case ctlparser_res_finished: return ctlparser_s_fin;

    case ctlparser_res_want_more: return ctlparser_s_reading_block;
    case ctlparser_res_want_to_dispose: return  ctlparser_s_disposing_block;

    case ctlparser_res_undef:
    default:
        log_msg(llv_alert, 
                "res2state: got undef result (BUG)");
        return ctlparser_res_undef;
    }
}

static int txt_parse_bind_ccmd(struct ms_cparser* cp, const char* args) {
    struct ms_ctl_session* ses = cp->the_session;
    int bind_iport = ms_conn_iport_undef;
    cp->target->type = ccmd_bind;

    if(sscanf(args, " %d ", &bind_iport) < 1) {
        log_msg(llv_debug, 
                "%s: BIND command doesn't have a valid argument",
                ses_description(ses));
        return ctlparser_res_error;
    }
    cp->target->u.bind.iport = bind_iport;
    return ctlparser_res_finished;
}

static int txt_parse_chmod_ccmd(struct ms_cparser* cp, const char* args) {
    struct ms_ctl_session* ses = cp->the_session;
    struct ms_ccmd* tar = cp->target;
    char mode_str[16];

    if(sscanf(args, " %15s ", mode_str) < 1) {
        log_msg(llv_debug, 
                "%s: CHMOD command doesn't have a valid argument",
                ses_description(ses));
        return ctlparser_res_error;
    }
    tar->type = ccmd_chmod;
    memcpy(tar->u.chmod.new_mode_str, mode_str, mode_str_max_len);
    tar->u.chmod.new_mode_str[mode_str_max_len] = 0;
    return ctlparser_res_finished;
}

static int txt_parse_close_ccmd(struct ms_cparser* cp, const char* args) {
    struct ms_ccmd* tar = cp->target;

    tar->type = ccmd_close;
    tar->u.close.code = 0;
    return ctlparser_res_finished;
}

typedef int (*ccmd_cb)(struct ms_cparser*, const char*);

enum txt_ccmd_type {
    tcmd_main = 0,
    tcmd_sub = 128,
    tcmd_opt = 256,
};

struct txt_ccmd {
    int ccmd;
    enum txt_ccmd_type type;
    char* name;
    ccmd_cb cb;
};

static struct txt_ccmd txt_ccmds[] = {
    {ccmd_bind,  tcmd_main,          "BIND",  &txt_parse_bind_ccmd},
    {ccmd_chmod, tcmd_main,          "CHMOD", &txt_parse_chmod_ccmd},
    {ccmd_stat,  tcmd_main,          "STAT",  NULL},

    {ccmd_send,  tcmd_main,          "SEND",  NULL},
    {ccmd_send,  tcmd_sub,           "DATA",  NULL},
    {ccmd_send,  tcmd_opt,           "LEN",   NULL},

    {ccmd_close, tcmd_main,          "CLOSE", &txt_parse_close_ccmd},
};

static struct trie cmd_trie;

struct ms_ccmd* make_cmd()
{
    struct ms_ccmd* res = malloc(sizeof(*res));
    memset(res, 0, sizeof(*res));
    res->type = ccmd_undef;
    res->parse_res = ctlparser_res_undef;
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
                    struct ms_ctl_session* ses,
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
    case ccmd_undef: 
        break;
    case ccmd_bind: 
        cmd->u.bind.iport = ms_conn_iport_undef;
        break;
    case ccmd_chmod: 
        cmd->u.chmod.new_mode_str[0] = 0;
        break;
    case ccmd_stat: 
        cmd->u.stat.addr.addr_resolve_type = stat_resolve_undef;
        break;
    case ccmd_send: 
        cmd->u.send.addr.addr_resolve_type = stat_resolve_undef;
        if(cmd->u.send.body)
            free(cmd->u.send.body);
        cmd->u.send.body = NULL;
        cmd->u.send.body_bytes_read = 0;
        cmd->u.send.body_len = 0;
        break;
    case ccmd_close: 
        cmd->u.close.code = 0;
        break;
    default:
        log_msg(llv_alert, " cleanup_cmd: unexpected value for control command type (%d) (BUG)", cmd->type);
        break;
    }

}


void cmd_init(struct ms_ccmd* cmd, int type)
{
    cleanup_cmd(cmd);
    cmd->type = type;
    cmd->parse_res = ctlparser_res_undef;
}

void dispose_cmd(struct ms_ccmd* cmd)
{
    cleanup_cmd(cmd);
    free(cmd);
}

void ms_cparser_reset(struct ms_cparser* cp)
{
    cp->state = ctlparser_s_init;
    if(cp->target)
        cmd_init(cp->target, ccmd_undef);
}

static const char* mode2a (enum ms_ctlparser_mode mode) {
    switch (mode) {
    case ctlparser_m_undef:  return "UNDEF";
    case ctlparser_m_text:   return "TEXT";
    case ctlparser_m_binary: return "BINARY";
    }
    return "UNKNOWN";
}

void ms_cparser_switch_mode(struct ms_cparser* cp, int new_mode)
{
    struct ms_ctl_session* ses = cp->the_session;
    log_msg(llv_debug, 
            "%s: switching mode to %s",
            ses_description(ses), mode2a(new_mode));
    cp->mode = new_mode;
}

void ms_cparser_cleanup(struct ms_cparser* cp)
{
    /* that's good */
}

static long long find_block(struct ms_cparser* cp) {
    while(cp->buf_p < cp->buf_used) 
    {
        if(cp->buf[cp->buf_p] == '\n' &&
           cp->buf_p > 0 &&
           cp->buf[cp->buf_p - 1] == '\n') 
        {
            cp->buf_p++;
            return cp->buf_p;
        }
        cp->buf_p++;
    }
    return 0;
}


static int inner_parse_txt(struct ms_cparser* parser,
                           long long* readed)
{
    struct ms_ctl_session* ses = parser->the_session;
    ccmd_cb cb;
    int cmd_len, res, block_len;

    *readed = find_block(parser);
    if(!*readed)
        return ctlparser_res_want_more;

    block_len = parser->buf_p - 2; 
    if(block_len <= 0) {
        log_msg(llv_alert, "empty control command block");
        return ctlparser_res_error;
    }
    
    parser->buf[block_len] = 0;
    cb = trie_get_with_len(&cmd_trie, (char*)parser->buf, &cmd_len);
    if(!cb) {
        log_msg(llv_debug, 
                "%s: unknown control command \"%s\"", 
                ses_description(ses), parser->buf);
        return ctlparser_res_error;
    }
    res = (*cb)(parser, (char*)parser->buf + cmd_len);
    parser->target->parse_res = res;
    switch (res) {
    case ctlparser_res_fatal:
        parser->state = ctlparser_s_fin_fatal;
        return res;
    case ctlparser_res_error:
        parser->state = ctlparser_s_fin_error;
        return res;
    case ctlparser_res_finished:
        parser->state = ctlparser_s_fin;
        return res;
    default:
        return res;
    }

    
}

int ms_cparser_read(struct ms_cparser* cp, int fd)
{
    long long rd;
    long long parser_readed;
    int parser_res = ctlparser_res_fatal;

    switch (cp->state) {
    case ctlparser_s_fin:
    case ctlparser_s_fin_error:
    case ctlparser_s_fin_fatal:
        log_msg(llv_alert, "ms_cparser_read called with %s state (BUG)", parser_state2a(cp->state));
        return cp->state;

    case ctlparser_s_init:
        cp->state = ctlparser_s_reading_block;
    }

    if (cp->buf_used < sizeof(cp->buf)) {
        rd = read(fd, cp->buf + cp->buf_used, sizeof(cp->buf) - cp->buf_used);
        if (rd <= 0) {
            if(rd == -1)
                log_perror(llv_alert, "ms_cparser_read", "read");
            return ctlparser_res_conn_closed; 
        }
        cp->buf_used += rd;
    }

    parser_readed = 0;
    switch (cp->mode) {
    case ctlparser_m_text:
        return inner_parse_txt(cp, &parser_readed);
    case ctlparser_m_binary:
        log_msg(llv_alert, "BINARY mode not supported yet");
        cp->state = ctlparser_s_fin_fatal;
        return ctlparser_res_fatal;
    default: 
        log_msg(llv_alert, "unexpected value for parser mode (%d) (BUG)", cp->mode);
        parser_res = ctlparser_res_fatal;
    }

    if(parser_readed > 0) {
        memmove(cp->buf, cp->buf + parser_readed, cp->buf_used - parser_readed);
        cp->buf_used -= parser_readed;
        cp->buf_p -= parser_readed;
    }
    return parser_res;
}
