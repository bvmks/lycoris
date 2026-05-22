#ifndef _MS_COMM_HANDLERS_H
#define _MS_COMM_HANDLERS_H

struct ms_ccmd;
struct ms_cparser;
struct ms_ctl_session;
struct ms_ccmd_result;

enum ms_ccmd_result_code {
    ccmd_rc_undef = -1,
    ccmd_rc_ok = 0,              /* as it said `all goochi` */

    ccmd_rc_parse_error = 101,   /* parsing failed */

    ccmd_rc_iarg = 201,          /* arguments is invalid */
    ccmd_rc_opdeny = 202,        /* operation denied */
};

int txt_send_code(struct ms_ctl_session* ses, int code);

int send_response(struct ms_ctl_session* ses, struct ms_ccmd_result* res);

struct ms_ccmd_result* process_ccmd(struct ms_cparser* cp, struct ms_ccmd* cmd);

void dispose_cmd_res(struct ms_ccmd_result* cres);

#endif
