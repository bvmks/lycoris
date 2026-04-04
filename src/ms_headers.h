#ifndef _MS_HEADERS_H
#define _MS_HEADERS_H

#include "ms_msg_types.h"
/*
 *  basic MS header
 *  when sending bytes must be in network order
 */
struct ms_header {
    struct ms_type type;
    unsigned short s_id; /* session ID */
    unsigned short seq; /* sequence num */
};


void header_init(struct ms_header* h);

/*
 * converts header bytes from host to network order
*/
void hton_header(struct ms_header* dst, const struct ms_header* src);

/*
 * converts header bytes from network to host order
*/
void ntoh_header(struct ms_header* dst, const struct ms_header* src);



#endif 
