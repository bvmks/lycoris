#ifndef _MS_HEADERS_H
#define _MS_HEADERS_H

struct ms_header_type {
    unsigned char t; /* msg type */
    unsigned char o; /* ctrl aka option */
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
_MS_T_IN(msc_conf)\
_MS_T_IN(msc_ack)\
_MS_T_IN(msc_deny)\
_MS_T_IN(msc_e_req)\
_MS_T_IN(msc_e_rep)



/* message types */
enum ms_h_type {
    /* mst = MS type */
    _MS_TYPES_LIST
};

/* control options for mst_ctrl message type*/
enum ms_h_opt {
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

enum {
    ptype_buf_len = 60,
};

char* ms_type2a(struct ms_header_type);


/*
 *  basic MS header
 *  when sending bytes must be in network order
 */

#define _MS_HEADERS_COMMON(name) struct ms_header_type name;

struct ms_header {
    _MS_HEADERS_COMMON(type);
};

struct ms_post_header {
    _MS_HEADERS_COMMON(type);
    unsigned short s_id; /* session ID */
    unsigned short seq; /* sequence num */
};

struct ms_stream_header {
    _MS_HEADERS_COMMON(type);
};


struct ms_ereq_header {
    _MS_HEADERS_COMMON(type);
    
};

struct ms_erep_header {
    _MS_HEADERS_COMMON(type);
};

#undef _MS_HEADERS_COMMON

enum {
    ms_gen_header_size = sizeof(struct ms_header),
    ms_post_header_size = sizeof(struct ms_post_header),
    ms_stream_header_size = sizeof(struct ms_stream_header),
    ms_ereq_header_size = sizeof(struct ms_ereq_header),
    ms_erep_header_size = sizeof(struct ms_erep_header),
};

/*
 * inits header type and opt fields
 */
void header_init(struct ms_header* h);

/*
 * mallocs and inits designated header type
 */
struct ms_header* header_make(unsigned char type, unsigned char opt);

/*
 * copies header AS IS, as for now no deep copy (and won't be (i guess))
 */
void header_copy(struct ms_header** dst, const struct ms_header* src);

/*
 * just moves pointer, src must'n be used anymore
 */
void header_move(struct ms_header** dst, struct ms_header* src);

/*
 * just frees mem
 */
void header_free(struct ms_header* h);

/*
 * packs header into memory, all numeric fields converts IN NETWORK BYTE order
 * any buffers
 */
int header2mem(void* dst, const struct ms_header* src);

/*
 * unpacks header from memory, converts all numeric fields into HOST BYTE order
 */
int mem2header(struct ms_header* dst, const void* src);

/*
 * converts header bytes from host to network order
*/
void hton_header(struct ms_header* dst, const struct ms_header* src);

/*
 * converts header bytes from network to host order
*/
void ntoh_header(struct ms_header* dst, const struct ms_header* src);

int get_header_size(const struct ms_header* h);

#endif 
