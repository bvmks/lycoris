#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ms_ctl.h"
#include "log.h"
#include "ms_ctlpar.h"
#include "ms_ctlbin.h"
#include "ms_ctltxt.h"

int ms_cparser_init(struct ms_cparser* cp, struct ms_ctl_session* ses)
{
    memset(cp, 0, sizeof(*cp));
    cp->the_session = ses;
    cp->binary_mode = 0;
    ms_cparser_reset(cp);
    return 1;
}

void ms_cparser_cleanup(struct ms_cparser* cp)
{
    if(cp->cur_cmd.type == ccmd_send && cp->cur_cmd.u.send.body) {
        free(cp->cur_cmd.u.send.body);
        cp->cur_cmd.u.send.body = NULL;
    }
}

void ms_cparser_reset(struct ms_cparser* cp)
{
    ms_cparser_cleanup(cp);
    cp->cur_cmd.type = ccmd_undef;
    cp->state = cps_reading_prefix;
    cp->bin_wanted_len = 2;
}

int ms_ctlparser_read(struct ms_cparser* cp)
{
    unsigned int rd, used;
    int parser_res;

    if(cp->state == cps_fin) {
        log_msg(llv_alert, "ms_ctlparser_read: called with 'fin' state (BUG)");
        return cp->state;
    }

    if(cp->buf_filled < sizeof(cp->buf)) {
        rd = read(cp->the_session->fdh.fd, 
                  cp->buf + cp->buf_filled, sizeof(cp->buf) - cp->buf_filled);
        if(rd <= 0) {
            if(rd == -1) {
                log_perror(llv_alert, "ms_ctlparser_read", "read");
            }
            return cpres_conn_closed; 
        }
        cp->buf_filled += rd;
    }
    
    if(cp->binary_mode)
        parser_res = binary_read(cp, &used);
    else
        parser_res = txt_read(cp, &used);

    if(used > 0) {
        memmove(cp->buf, cp->buf + used, cp->buf_filled - used);
        cp->buf_filled -= used;
        cp->buf_rd_pos -= used;
    }

    if(cp->buf_filled == sizeof(cp->buf) && parser_res == cpres_want_more) {
        log_msg(llv_alert, "%s: ms_cparser_read: buffer overflow!", 
                ses_description(cp->the_session));
        return cpres_fatal;
    }

    return parser_res;
}



