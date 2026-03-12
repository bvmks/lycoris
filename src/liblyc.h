/*
 * 
 *
 *
 * 
 */



#ifndef _LIBLYC_H
#define _LIBLYC_H

#include <time.h>

#define _LIBLYC_TMST_T unsigned long
#define _LIBLYC_MESG_T unsigned short
#define _LIBLYC_HASH_T int



#define _LYC_HEADER_IN(type, name) type name;

#define _HEADER_LIST \
        _LYC_HEADER_IN(_LIBLYC_MESG_T, type)\
        _LYC_HEADER_IN(_LIBLYC_HASH_T, hash)\
        _LYC_HEADER_IN(_LIBLYC_TMST_T, timestamp)

/*
 *  basic MS protocol header
 */
struct ms_msg_header {
    _HEADER_LIST
};


#undef _LYC_HEADER_IN 
#define _LYC_HEADER_IN(type, ...) sizeof(type) +

struct ipv4_address {
    unsigned short  port;
    unsigned int    address;
};

enum {
    ms_header_len = (_HEADER_LIST 0)
};

enum ms_msg_type {
    message = 0
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
_LIBLYC_TMST_T get_timestamp();


#endif // !_LIBLYC_H
