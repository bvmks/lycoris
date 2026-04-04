#ifndef _SOCKS_H
#define _SOCKS_H

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

/*
*  automatically creates socket with AF_INET and type
*  and binds it to specified ip and port
*  returns fd, on error returns -1 then use errno
*  addr and port must be in host byte order!
*/
int make_sock(int type, unsigned int addr, unsigned short port);


#endif
