#ifndef _MS_MSGT_H
#define _MS_MSGT_H

struct ms_type {
    unsigned char type; /* msg type */
    unsigned char opt; /* ctrl aka option */
};

#define _MS_T_IN(name) name,

#define _MS_TYPES_LIST \
_MS_T_IN(mst_err)\
_MS_T_IN(mst_post)\
_MS_T_IN(mst_stream)\
_MS_T_IN(mst_ctrl)

#define _MS_CTRL_TYPES_LIST \
_MS_T_IN(msc_err)\
_MS_T_IN(msc_init)\
_MS_T_IN(msc_term)\
_MS_T_IN(msc_heartbeat)\
_MS_T_IN(msc_ack)\
_MS_T_IN(msc_deny)



/* message types */
enum ms_msg_type {
    /* mst = MS type */
    _MS_TYPES_LIST
};

/* control options for mst_ctrl message type*/
enum ms_ctrl {
    /* msc = MS control option*/
    _MS_CTRL_TYPES_LIST
};

#undef _MS_T_IN
#define _MS_T_IN(...) + 1

enum {
    ms_msg_types_num = 0 _MS_TYPES_LIST,
    ms_msg_ctrl_types_num = 0 _MS_CTRL_TYPES_LIST
};

#undef _MS_T_IN

char* ms_type2a(struct ms_type);

#endif
