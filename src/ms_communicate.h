#ifndef _MS_COMMUNICATE_H
#define _MS_COMMUNICATE_H

#include <sys/socket.h>


#include "ms_packets.h"
#include "ms_headers.h"

enum {
    ms_packet_max_size = 508,
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
int ms_send(int sockfd, struct sockaddr_in* dst, struct ms_packet* header);

/*
*   receives the msg and stores it in 
*   pointer to struct ms_packet
*   returns 0 if everything ok, otherwise returns -1
*/
int ms_recv(int sockfd, struct ms_packet* in_packet);

#endif
