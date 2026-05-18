#ifndef _MS_COMM_HANDLERS_H
#define _MS_COMM_HANDLERS_H

struct ms_ccmd;

struct ms_ccmd_result {

};

struct ms_ccmd_result* handle_ccmd(struct ms_ccmd* cmd);

void dispose_cmd_res(struct ms_ccmd_result* cres);

#endif
