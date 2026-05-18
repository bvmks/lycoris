#include <stdlib.h>
#include <string.h>
#include "ms_cparser.h"
#include "log.h"

static void* cmd_marker_bind = (void*)1;
static void* cmd_marker_stat = (void*)2;

struct ms_ccmd* make_cmd()
{
    struct ms_ccmd* res = malloc(sizeof(*res));
    memset(res, 0, sizeof(*res));
    return res;
}

void cmd_reset(struct ms_ccmd* cmd)
{
    if (cmd->body_buf) {
        free(cmd->body_buf);
    }
    memset(cmd, 0, sizeof(*cmd));
}

void dispose_cmd(struct ms_ccmd* cmd)
{
    if (cmd->body_buf) {
        free(cmd->body_buf);
    }
    free(cmd);
}

int ms_cparser_init(struct ms_cparser* cp, int start_mode)
{
    memset(cp, 0, sizeof(*cp));
    trie_init(&cp->cmd_trie);
    
    void** slot;
    
    slot = trie_provide(&cp->cmd_trie, "BIND");
    if (slot) *slot = cmd_marker_bind;
    
    slot = trie_provide(&cp->cmd_trie, "STAT");
    if (slot) *slot = cmd_marker_stat;
    
    cp->mode = start_mode;
    ms_cparser_reset(cp);
   
    return 1;
}

void ms_cparser_reset(struct ms_cparser* cp)
{
    cp->state = ms_cps_init;
    cp->buf_used = 0;
    cp->direct_wanted_bytes = 0;
    cp->direct_ptr = NULL;
}

void ms_cparser_switch_mode(struct ms_cparser* cp, int new_mode)
{
    if (cp) cp->mode = new_mode;
}

void* ms_cparser_get_direct(struct ms_cparser *cp)
{
    if (cp->state != ms_cps_reading_body) return NULL;
    return cp->direct_ptr;
}

unsigned long long ms_cparser_want_direct(struct ms_cparser *cp)
{
    if (cp->state != ms_cps_reading_body) return 0;
    return cp->direct_wanted_bytes;
}

void ms_cparser_cleanup(struct ms_cparser* cp)
{
    trie_clear(&cp->cmd_trie);
}

int ms_cparser_feed(struct ms_cparser* cp, struct ms_ccmd* cmd,
                    unsigned char* data,
                    unsigned long long datalen, long long* read,
                    long long int direct)
{
    /*TODO: YYYYYYYYYYYYYYY make this shit plz*/

}
