#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "socks.h"
#include "message.h"

int make_sock(int type, unsigned int addr, unsigned short port)
{
    struct sockaddr_in own_addr;
    int sockfd, ok;

    own_addr.sin_family = AF_INET;
    own_addr.sin_port = htons(port);
    own_addr.sin_addr.s_addr = htonl(addr);
    
    sockfd = socket(AF_INET, type, 0);
    if(sockfd == -1) 
        goto error;
    ok = bind(sockfd, (struct sockaddr*)&own_addr, sizeof(own_addr));
    if(ok == -1) 
        goto error;

    return sockfd;

error: {
        close(sockfd);
        return -1;
    }
}
