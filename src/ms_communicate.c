#include "ms_communicate.h"

int ms_send(int sockfd, struct ipv4_address *dst, unsigned short msg_type, char *msg, int msg_len)
{
    struct ms_msg_header h;
    h.type = msg_type;
    h.hash = 

}
