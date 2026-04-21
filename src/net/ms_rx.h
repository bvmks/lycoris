#ifndef _MS_RX_H
#define _MS_RX_H

int send_to(int fd, unsigned int ip, unsigned short port,
             const void *buf, int len);


#endif
