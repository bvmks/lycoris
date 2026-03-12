/*
 * 
 *
 *
 * 
 */



#ifndef _LIBLYC_H
#define _LIBLYC_H

#include <time.h>

#define _MS_TMST_T unsigned long
#define _MS_TYPE_T unsigned short
#define _MS_CTRL_T unsigned short
#define _MS_HASH_T int

#define _LYC_HEADER_IN(type, name) type name;


#define _GEN_HEADER_LIST \
        _LYC_HEADER_IN(_MS_TYPE_T, type)

#define _MSG_HEADER_LIST \
        _LYC_HEADER_IN(_MS_TYPE_T, type)\
        _LYC_HEADER_IN(_MS_TMST_T, timestamp)

#define _CTR_HEADER_LIST \
        _LYC_HEADER_IN(_MS_TYPE_T, type)\
        _LYC_HEADER_IN(_MS_CTRL_T, ctrl)

/*
 *  basic MS header
 */
struct ms_basic_header {
    _GEN_HEADER_LIST
};

struct ms_msg_header {
    _MSG_HEADER_LIST
};

struct ms_ctrl_header {
    _CTR_HEADER_LIST
};

#undef _LYC_HEADER_IN 
#define _LYC_HEADER_IN(type, ...) sizeof(type) +

struct ipv4_address {
    unsigned short  port;
    unsigned int    address;
};

enum {
    ms_basic_header_len = (_GEN_HEADER_LIST 0),
    ms_msg_header_len =   (_MSG_HEADER_LIST 0),
    ms_ctr_header_len =   (_CTR_HEADER_LIST 0),
};

enum ms_msg_type {
    message = 0,
    ctrl = 1
};

enum ms_ctrl_cmd {
    acknowledge = 0
};

/*
 *  serializes MS header into memory region
 *  at least expected sizeof(ms_msg_header) bytes of memory
 *  final amount of written bytes may be less
*/
int ms_pack_header(struct ms_msg_header* src, void* dst);

/*
 *  deserializes MS header from memory region into structure
*/
int ms_unpack_header(void* src, struct ms_msg_header* dst);


/*
 *  returns time in milliseconds since Epoch
*/
_MS_TMST_T get_timestamp();


#endif // !_LIBLYC_H
