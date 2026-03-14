#ifndef _MS_COMMUNICATE_H
#define _MS_COMMUNICATE_H

#include <sys/socket.h>


#include "ms_packets.h"
#include "ms_headers.h"

enum {
    dgram_max_size = 508,
    ms_header_size = sizeof(struct ms_header),
    ms_packet_hash_size = 4,
    ms_packet_payload = dgram_max_size 
                           - ms_header_size
                           - ms_packet_hash_size,
};

/*
 *  checks standart frame for integrity
 *  expects last 4 bytes to be a crc32 hash of all frame
 *  returns 0 on success, -1 otherwise
 */
int ms_crc32_check(char* buf, int buf_len);

/*
*   sends ms_packet 
*   returns amount of sent bytes, -1 if error occured
*/
int ms_send(int sockfd, struct addrport* dst, struct ms_packet* packet);

/*
*   receives the msg and stores it in 
*   pointer to struct ms_packet
*   returns 0 if everything ok, otherwise returns -1
*/
int ms_recv(int sockfd, struct ms_received_packet* in_packet);

#endif
