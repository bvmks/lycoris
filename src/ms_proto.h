#ifndef _MS_PROTOCOL_H
#define _MS_PROTOCOL_H

#include "ms_packets.h"
#include "ms_session.h"
#include "ms_connection.h"
#include "ms_msg_types.h"
#include "addrport.h"
#include "ms_communicate.h"

/*
 * sends control packet
 * from enum ms_ctrl
 */
int ms_send_ctrl(int sockfd, struct ms_connection* p, unsigned short opt);


int ms_send_confirm(int sockfd, struct ms_connection* p, unsigned short seq);

/*
*  automatically sends header + buffer + hash 
*  header will be filled with session id, current seq and type/opt specified
*  increases session seq num and places packet into perr sent list
*/
int ms_send(int sockfd,
            struct ms_connection* c,
            unsigned short type,
            unsigned short opt,
            const char* buf,
            int buf_len);


int ms_recv_packet(int sockfd, struct ms_received_packet* p);


#endif
