#ifndef _MS_HEADERS_H
#define _MS_HEADERS_H

/*
 *  basic MS header
 *  when sending bytes must be in network order
 */
struct ms_header {
    struct ms_type {
        unsigned char type; /* msg type */
        unsigned char ctrl; /* ctrl aka option */
    } type;
    unsigned short s_id; /* session ID */
    unsigned short seq; /* sequence num */
};

/* message types */
enum ms_msg_type {
    /* mst = MS type */
    mst_post = 1, /* msg with confirmation */
    mst_stream, /* stream part, no confirmation */
    mst_ctrl /* control message (may be with confirmation) */
};

/* control options for mst_ctrl message type*/
enum ms_ctrl {
    /* msc = MS control option*/
    msc_init = 1, /* initiating session, expects ack in return*/
    msc_heardbeat, /* to confirm that we still listening stream */
    msc_ack, /* to confirm*/
    msc_deny /* to deny*/
};


#endif 
