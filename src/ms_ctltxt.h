#ifndef _MS_TXTCON_PARSER_H
#define _MS_TXTCON_PARSER_H

struct ms_cparser;
struct ms_ctl_session;

void send_commit(struct ms_ctl_session *ses);

int txt_read(struct ms_cparser* cp, unsigned int* used);

#endif
