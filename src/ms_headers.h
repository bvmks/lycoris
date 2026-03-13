/*
 * 
 *
 *
 * 
 */

#ifndef _MS_HEADERS_H
#define _MS_HEADERS_H
#include <time.h>

/*
 *  basic MS header
 *  when sending bytes must be in network order
 */
struct ms_type {
    /* msg type */
    unsigned char type;
    /* ctrl aka option */
    unsigned char ctrl;
};
struct ms_header {
    struct ms_type type;
    /* session ID */
    unsigned short s_id;
    /* sequence num */
    unsigned short seq;
};

struct ipv4_address {
    unsigned short  port;
    unsigned int    address;
};

/* message types */
enum ms_msg_type {
    /* mst = MS type */
    /* msg with confirmation */
    mst_post = 1,
    /* stream part, no confirmation */
    mst_stream,
    /* control message (may be with confirmation) */
    mst_ctrl
};

/* control options for mst_ctrl message type*/
enum ms_ctrl {
    /* msc = MS control option*/
    /* initiating session, expects ack in return*/
    msc_init = 1,
    /* to confirm that we still listening stream */
    msc_heardbeat,
    /* to confirm*/
    msc_ack,
    /* to deny*/
    msc_deny
};


#endif 
