#include "ms_ctlpar.h"
#include "ms_ctl.h"
#include "log.h"

static void parse_bind(struct ms_cparser* cp)
{
    unsigned int* rd_pos = &cp->buf_rd_pos;
    unsigned char* buf = cp->buf;
    cp->cur_cmd.u.bind.iport = buf[*rd_pos];
}

static void bin_handle_ccmd(struct ms_cparser *cp)
{
    switch (cp->cur_cmd.type) {
    case ccmd_bind:
        parse_bind(cp);
        return;
    case ccmd_stat:
    case ccmd_send:
    case ccmd_recv:
        return;
    case ccmd_close:
        return;
    default:
    case ccmd_undef:
        return;
    }
}

static unsigned int wanted_from_type(int type)
{
    enum ccmd_type a = type;
    switch (a) {
    case ccmd_bind: return ccmd_bind_hlen;
    case ccmd_stat: return ccmd_stat_hlen;
    case ccmd_send: return ccmd_send_hlen;
    case ccmd_recv: return ccmd_recv_hlen;
    case ccmd_close: return ccmd_close_hlen;

    default:
    case ccmd_undef:
        return -1;
    }
}


static int parse_prefix(struct ms_cparser* cp, unsigned int* used)
{
    unsigned int* rd_pos = &cp->buf_rd_pos;
    unsigned char* buf = cp->buf;
    unsigned int want;

    cp->cur_cmd.type = buf[*rd_pos];
    *rd_pos += 1;
    cp->cur_cmd.status = buf[*rd_pos];
    *rd_pos += 1;
    *used = 2; 

    cp->state = cps_reading_header;
    want = wanted_from_type(cp->cur_cmd.type);
    if(want == -1) {
        log_msg(llv_debug,
                "%s: unknown control command type in prefix, closing session",
                ses_description(cp->the_session));
        return cpres_fatal;
    }
    cp->wanted_len = want;
    return cpres_continue;
}

static int parse_header(struct ms_cparser* cp, unsigned int* used)
{
    switch (cp->cur_cmd.type) {
    case ccmd_bind:
        parse_bind(cp);
        return cpres_finished;
    case ccmd_stat:
    case ccmd_send:
    case ccmd_recv:
        return cpres_fatal;
    case ccmd_close:
        return cpres_finished;
    default:
    case ccmd_undef:
        return cpres_fatal;
    }
}


static int parse_data(struct ms_cparser* cp, unsigned int* used)
{

    return cpres_finished;
}

static int bin_parse_block(struct ms_cparser* cp, unsigned int* used)
{
    unsigned int available = cp->buf_filled - cp->buf_rd_pos;

    if(available < cp->wanted_len)
        return cpres_want_more;

    switch (cp->state) {
    case cps_fin:
        return cp->state;
    case cps_reading_prefix:
        return parse_prefix(cp, used);
    case cps_reading_header:
        return parse_header(cp, used);
    case cps_reading_data:
        return parse_data(cp, used);
    break;
    }

    return cpres_fatal;
}

int binary_read(struct ms_cparser* cp, unsigned int* used)
{
    unsigned int u = 0;
    unsigned int total_used = 0;
    int parser_res = cpres_undef;
    int parsing_blocks = 1;

    while (parsing_blocks) {
        parser_res = bin_parse_block(cp, &u);
        total_used += u;

        switch (parser_res) {
        case cpres_error:
        case cpres_fatal:
        case cpres_want_more:
            parsing_blocks = 0;
            break;
        case cpres_continue:
            break;
        case cpres_finished:
            bin_handle_ccmd(cp);
            ms_cparser_reset(cp);
            break;
        }
    }
    *used = total_used;
    return parser_res;
}
