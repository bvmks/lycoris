#ifndef _MS_RX_H
#define _MS_RX_H

#include "ms_node.h"

int send_to(int fd, unsigned int ip, unsigned short port,
             const void *buf, int len);


int do_recv(struct ms_node*);

#endif
