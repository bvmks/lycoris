#ifndef _MS_COMM_HANDLERS_H
#define _MS_COMM_HANDLERS_H

#include "ms_con.h"

struct ms_ccmd;
struct ms_cparser;

enum ms_ccmd_result_code {
    ms_ccmd_rc_undef = -1,
    ms_ccmd_rc_ok = 0,              /* as it said `all goochi` */

    ms_ccmd_rc_parse_error = 101,   /* parsing failed */

    ms_ccmd_rc_iarg = 201,          /* arguments is invalid */
    ms_ccmd_rc_opdeny = 202,        /* operation denied */
};

struct ms_ccmd_result {
    enum ms_ccmd_type type;
    union {

    }u;

    int code;
};

int send_code(struct ms_con_session* ses, int code);

int send_response(struct ms_con_session* ses, struct ms_ccmd_result* res);

struct ms_ccmd_result* process_ccmd(struct ms_cparser* cp, struct ms_ccmd* cmd);

void dispose_cmd_res(struct ms_ccmd_result* cres);

#endif
