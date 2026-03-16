#ifndef _MS_COMMUNICATE_H
#define _MS_COMMUNICATE_H

#include <sys/socket.h>


#include "ms_packets.h"
#include "ms_headers.h"

enum {
    dgram_max_size = 508,
    ms_header_size = sizeof(struct ms_header),
    ms_hash_size = sizeof(unsigned int),
    ms_packet_payload = dgram_max_size 
                        - ms_header_size
                        - ms_hash_size,
    dgram_min_size = ms_header_size + ms_hash_size,
};

/*
 *  checks standart frame for integrity
 *  expects last 4 bytes to be a crc32 hash of all frame
 *  returns 0 on success, -1 otherwise
 */
int ms_crc32_check(char* buf, int buf_len);

/*
*   packs and sends header + data + hash
*   returns amount of sent bytes, -1 if error occured
*   data can be NULL and data_len = 0
*   header must BE
*/
int ms_send_raw(int sockfd, struct addrport* dst_addr,
                struct ms_header* header,
                const char* data, int data_len);

/*
*   sends ms_packet 
*   returns amount of sent bytes, -1 if error occured
*/
int ms_send(int sockfd, struct addrport* dst_addr, struct ms_packet* packet);


/*
*   receives the msg and stores it in 
*   pointer to struct ms_packet
*   returns 0 if everything ok, otherwise returns -1
*/
int ms_recv(int sockfd, struct ms_received_packet* in_packet);

#endif
