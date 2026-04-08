#ifndef _MS_COMMUNICATE_H
#define _MS_COMMUNICATE_H

#include "ms_packets.h"
#include "ms_headers.h"

enum {
    dgram_max_size = 508,
    ms_hash_size = sizeof(unsigned int),
    dgram_min_size = ms_gen_header_size + ms_hash_size,
    ms_ctrl_packet_size = dgram_min_size
};

enum ms_com_errors {
    ms_ce_msg_too_long = -2,
};

/*
 *  checks standart frame for integrity
 *  expects last 4 bytes to be a crc32 hash of all frame
 *  returns 0 on success, -1 otherwise
 */
int ms_crc32_check(const char* buf, int buf_len);

/*
*   packs and sends header + data + hash
*   returns amount of sent bytes, -1 if error occured
*   data can be NULL and data_len = 0
*   header must BE
*/
int ms_send_raw(int sockfd, 
                const struct addrport* dst_addr,
                const struct ms_header* header,
                const char* data,
                int data_len);

/*
*   sends ms_packet 
*   returns amount of sent bytes, -1 if error occured
*/
int ms_send_packet(int sockfd, 
                   const struct addrport* dst_addr,
                   const struct ms_packet* packet);

/*
 *
 *  parses buffer into header + data
 *  for data allocates new mem
 */
void ms_parse(const char* buf,
             int buf_len,
             struct ms_header** header,
             char** data,
             int* data_len);


#endif
