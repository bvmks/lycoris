#include <stdlib.h>
#include <string.h>

#include "ms_ctl.h"
#include "ms_ctlhdl.h"

#include "log.h"
#include "utils.h"

static struct ms_ccmd_result* make_result(int type, int code) 
{
    struct ms_ccmd_result* res = malloc(sizeof(*res));
    memset(res, 0, sizeof(*res));
    res->type = type;
    res->code = code;
    return res;
}

static struct ms_ccmd_result* handle_chmod(struct ms_cparser* cp, struct ms_ccmd* cmd) {
    struct ms_ctl_session* ses = cp->the_session;
    if(0 == strcmp(cmd->u.chmod.new_mode_str, "BINARY")) {
        ms_cparser_switch_mode(cp, ctlparser_m_binary);
        return make_result(ccmd_chmod, ccmd_rc_ok);
    }
    else if(0 == strcmp(cmd->u.chmod.new_mode_str, "TEXT")) {
        ms_cparser_switch_mode(cp, ctlparser_m_text);
        return make_result(ccmd_chmod, ccmd_rc_ok);
    }
    else {
        log_msg(llv_debug, 
                "%s: CHMOD ERR unknown mode (%s)",
                ses_description(ses), cmd->u.chmod.new_mode_str);
        return make_result(ccmd_chmod, ccmd_rc_iarg);
    }
}

static struct ms_ccmd_result* handle_close(struct ms_cparser* cp, struct ms_ccmd* cmd) {
    struct ms_ctl_session* ses = cp->the_session;

    ses->to_close = 1;

    log_msg(llv_debug, 
            "%s: session closed by other side, code %d",
            ses_description(ses), cmd->u.close.code);

    return make_result(cmd->type, ccmd_rc_ok);
}

static struct ms_ccmd_result* handle_bind(struct ms_cparser* cp, struct ms_ccmd* cmd) {
    struct ms_ctl_session* ses = cp->the_session;
    struct ms_ctl_session* bind_slot;
    int bind_port = cmd->u.bind.iport;

    if(ses->bound) {
        log_msg(llv_debug, 
                "%s: BIND ERR trying to rebind", ses_description(ses));
        return make_result(cmd->type, ccmd_rc_opdeny);
    }

    if(bind_port > ms_conn_iport_max || bind_port <= ms_conn_iport_all) {
        log_msg(llv_debug, 
                "%s: BIND ERR invalid port (%d)",
                ses_description(ses), bind_port);
        return make_result(cmd->type, ccmd_rc_iarg);
    }
    
    bind_slot = ses->master->ports[bind_port];

    if(bind_slot) {
        log_msg(llv_debug, 
                "%s: BIND ERR port (%d) already in use",
                ses_description(ses));
        return make_result(cmd->type, ccmd_rc_opdeny);
    }

    ses->master->ports[bind_port] = ses;
    ses->bound = 1;
    ses->iport = bind_port;

    log_msg(llv_debug, 
            "%s: bound to port (%d)",
            ses_description(ses), bind_port);

    return make_result(cmd->type, ccmd_rc_ok);
}

struct ms_ccmd_result* process_ccmd(struct ms_cparser* cp, struct ms_ccmd* cmd)
{
    struct ms_ctl_session* ses = cp->the_session;
    if(cmd->parse_failed) {
        return NULL;
    }

    switch (cmd->type) {
    case ccmd_undef:
        log_msg(llv_alert, 
                "%s: handle_ccmd got undefined cmd (BUG)",
                ses_description(ses));
        break;
    case ccmd_bind:
        log_msg(llv_debug, 
                "%s: got BIND control command (port=%d)",
                ses_description(ses),
                cmd->u.bind.iport);
            return handle_bind(cp, cmd);
        break;
    case ccmd_chmod:
        log_msg(llv_debug,
                "%s: got CHMOD control command (mode=%s)",
                ses_description(ses),
                cmd->u.chmod.new_mode_str);
            return handle_chmod(cp, cmd);
        break;
    case ccmd_stat:
    case ccmd_send:
        break;
    case ccmd_close:
        log_msg(llv_debug,
                "%s: got CLOSE control command (code=%d)",
                ses_description(ses),
                cmd->u.close.code);
            return handle_close(cp, cmd);
        break;
    }
    log_msg(llv_alert,
            "%s: handle_ccmd got unknown cmd (BUG)",
            ses_description(ses));
    return NULL;
}

void dispose_cmd_res(struct ms_ccmd_result* cres)
{
    /*TODO: of course it's no all...*/
    if(cres)
        free(cres);
}

static const char* rescode2a(enum ms_ccmd_result_code code)
{
    switch (code) {
    default:                    
    case ccmd_rc_undef:          return "";
    case ccmd_rc_ok:             return "OK";

    case ccmd_rc_parse_error:    
    case ccmd_rc_iarg:
    case ccmd_rc_opdeny:         return "ERROR";
    }
}


int txt_send_code(struct ms_ctl_session* ses, int code) {
    int r;
    r = fprintf(ses->stream, "%s %s\n\n", 
                rescode2a(code), code == 0 ? "" : decimal2a(code));
    fflush(ses->stream);
    return r;
}

static int txt_send_response(struct ms_ctl_session* ses, struct ms_ccmd_result* res)
{
    int r = 0;

    if(!res) {
        txt_send_code(ses, ccmd_rc_parse_error);
        return r;
    }

    switch (res->type) {
    case ccmd_undef:
        break;
    case ccmd_bind:
    case ccmd_chmod:
    case ccmd_close:
        txt_send_code(ses, res->code);
        break;
    case ccmd_stat:
    case ccmd_send:

        break;
    }
    return r;
}

int send_response(struct ms_ctl_session* ses, struct ms_ccmd_result* res)
{
    switch (ses->parser.mode) {
    default: 
    case ctlparser_m_undef:  return 0;
    case ctlparser_m_text:   return txt_send_response(ses, res);
    case ctlparser_m_binary: return 0;
        break;
    }
}
