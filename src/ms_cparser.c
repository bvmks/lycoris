#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ms_cparser.h"
#include "log.h"

static void* cmd_marker_bind = (void*)1;
static void* cmd_marker_stat = (void*)2;
static void* cmd_marker_send = (void*)3;

typedef int (*ccmd_cb)(struct ms_ccmd *, const char *, int);

struct ms_ccmd* make_cmd()
{
    struct ms_ccmd* res = malloc(sizeof(*res));
    memset(res, 0, sizeof(*res));
    return res;
}

int ms_cparser_init(struct ms_cparser* cp, struct ms_ccmd* target,
                    int start_mode)
{
    memset(cp, 0, sizeof(*cp));
    trie_init(&cp->cmd_trie);

    cp->target = target;
    
    void** slot;
    
    slot = trie_provide(&cp->cmd_trie, "BIND");
    *slot = cmd_marker_bind;
    
    slot = trie_provide(&cp->cmd_trie, "STAT");
    *slot = cmd_marker_stat;

    slot = trie_provide(&cp->cmd_trie, "SEND");
    *slot = cmd_marker_send;
    
    cp->mode = start_mode;
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
        log_msg(llv_alert, "unexpected value for ms_ccmd_type (%d) (BUG)", cmd->type);
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
    case ms_ccmd_stat: 
        break;
    case ms_ccmd_send: 
        break;
    default:
        log_msg(llv_alert, "unexpected value for ms_ccmd_type (%d) (BUG)", cmd->type);
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


static int ms_parse_binary(struct ms_cparser* cp, 
                           long long* readed)
{

}

static int ms_parse_text(struct ms_cparser* cp, 
                         long long* readed)
{

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

    parser_readed = 0;
    switch (cp->mode) {
    case ms_cpm_text:
        parser_res = ms_parse_text(cp, &parser_readed);
        break;
    case ms_cpm_binary:
        parser_res = ms_parse_binary(cp, &parser_readed);
        break;
    default: 
        log_msg(llv_alert, "unexpected value for ms_cpm (%d) (BUG)", cp->mode);
        parser_res = ms_cp_res_fatal;
    }

    if(parser_readed > 0) {
        memmove(cp->buf, cp->buf + parser_readed, cp->buf_used - parser_readed);
        cp->buf_used -= parser_readed;
    }
    return parser_res;
}
