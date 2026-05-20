#include <stdlib.h>
#include <string.h>

#include "ms_con.h"
#include "ms_chandl.h"
#include "ms_cparser.h"
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
    struct ms_con_session* ses = cp->the_session;
    if(0 == strcmp(cmd->u.chmod.new_mode_str, "BINARY")) {
        ms_cparser_switch_mode(cp, ms_cpm_binary);
        return make_result(ms_ccmd_chmod, ms_ccmd_rc_ok);
    }
    else if(0 == strcmp(cmd->u.chmod.new_mode_str, "TEXT")) {
        ms_cparser_switch_mode(cp, ms_cpm_text);
        return make_result(ms_ccmd_chmod, ms_ccmd_rc_ok);
    }
    else {
        log_msg(llv_debug, 
                "CONTROL SESSION [%d][%s]: CHMOD ERR unknown mode (%s)",
                ses->id, 
                ses->bound ? decimal2a(ses->iport) : "-",
                cmd->u.chmod.new_mode_str);
        return make_result(ms_ccmd_chmod, ms_ccmd_rc_iarg);
    }
}

static struct ms_ccmd_result* handle_close(struct ms_cparser* cp, struct ms_ccmd* cmd) {
    struct ms_con_session* ses = cp->the_session;

    ses->to_close = 1;

    log_msg(llv_debug, 
            "CONTROL SESSION [%d][%s]: session closed by other side, code %d",
            ses->id, 
            ses->bound ? decimal2a(ses->iport) : "-",
            cmd->u.close.code);

    return make_result(cmd->type, ms_ccmd_rc_ok);
}

static struct ms_ccmd_result* handle_bind(struct ms_cparser* cp, struct ms_ccmd* cmd) {
    struct ms_con_session* ses = cp->the_session;
    struct ms_con_session* bind_slot;
    int bind_port = cmd->u.bind.iport;

    if(ses->bound) {
        log_msg(llv_debug, 
                "CONTROL SESSION [%d][%s]: BIND ERR trying to rebind",
                ses->id, 
                ses->bound ? decimal2a(ses->iport) : "-");
        return make_result(cmd->type, ms_ccmd_rc_opdeny);
    }

    if(bind_port > ms_conn_iport_max || bind_port <= ms_conn_iport_all) {
        log_msg(llv_debug, 
                "CONTROL SESSION [%d][%s]: BIND ERR invalid port (%d)",
                ses->id, 
                ses->bound ? decimal2a(ses->iport) : "-",
                bind_port);
        return make_result(cmd->type, ms_ccmd_rc_iarg);
    }
    
    bind_slot = ses->the_master->ports[bind_port];

    if(bind_slot) {
        log_msg(llv_debug, 
                "CONTROL SESSION [%d][%s]: BIND ERR port (%d) already in use",
                ses->id, 
                ses->bound ? decimal2a(ses->iport) : "-",
                bind_port);
        return make_result(cmd->type, ms_ccmd_rc_opdeny);
    }

    ses->the_master->ports[bind_port] = ses;
    ses->bound = 1;
    ses->iport = bind_port;

    log_msg(llv_debug, 
            "CONTROL SESSION [%d][%s]: bound to port (%d)",
            ses->id, 
            ses->bound ? decimal2a(ses->iport) : "-",
            bind_port);

    return make_result(cmd->type, ms_ccmd_rc_ok);
}

struct ms_ccmd_result* process_ccmd(struct ms_cparser* cp, struct ms_ccmd* cmd)
{
    switch (cmd->type) {
    case ms_ccmd_undef:
        log_msg(llv_alert, "handle_ccmd got undefined cmd (BUG)");
        break;
    case ms_ccmd_bind:
        log_msg(llv_debug, "got BIND control command (port=%d)", cmd->u.bind.iport);
            return handle_bind(cp, cmd);
        break;
    case ms_ccmd_chmod:
        log_msg(llv_debug, "got CHMOD control command (mode=%s)", cmd->u.chmod.new_mode_str);
            return handle_chmod(cp, cmd);
        break;
    case ms_ccmd_stat:
    case ms_ccmd_send:
        break;
    case ms_ccmd_close:
        log_msg(llv_debug, "got CLOSE control command (code=%d)", cmd->u.close.code);
            return handle_close(cp, cmd);
        break;
    }
    log_msg(llv_alert, "handle_ccmd got unknown cmd (BUG)");
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
    case ms_ccmd_rc_undef:          return "";
    case ms_ccmd_rc_ok:             return "OK";

    case ms_ccmd_rc_parse_error:    
    case ms_ccmd_rc_iarg:
    case ms_ccmd_rc_opdeny:         return "ERROR";
    }
}


int send_code(struct ms_con_session* ses, int code) {
    int r;
    r = fprintf(ses->stream, "%s %s\n\n", 
                rescode2a(code), code == 0 ? "" : decimal2a(code));
    fflush(ses->stream);
    return r;
}

static int inner_send_txt(struct ms_con_session* ses, struct ms_ccmd_result* res)
{
    int r = 0;
    switch (res->type) {
    case ms_ccmd_undef:
        break;
    case ms_ccmd_bind:
    case ms_ccmd_chmod:
    case ms_ccmd_close:
        send_code(ses, res->code);
        break;
    case ms_ccmd_stat:
    case ms_ccmd_send:
        break;
    }
    return r;
}

int send_response(struct ms_con_session* ses, struct ms_ccmd_result* res)
{
    switch (ses->parser.mode) {
    default: 
    case ms_cpm_undef:  return 0;
    case ms_cpm_text:   return inner_send_txt(ses, res);
    case ms_cpm_binary: return 0;
        break;
    }
}
