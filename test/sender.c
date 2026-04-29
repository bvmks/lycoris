#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../src/socks.h"
#include "../src/addrport.h"
#include "../src/ms_rx.h"

enum {
    port = 24881
};

int main(int argc, char **argv)
{
    int sockfd, msg_len, ok;
    struct addrport dst;
    char buf[1000];
    // char buf[] =  "bebra";

    if (argc < 2) {
        printf("usage: %s ip:port\n", argv[0]);
        return 1;
    }
    
    str2addrport(&dst, argv[1]);
    sockfd = make_sock(SOCK_DGRAM, INADDR_ANY, port);

    for(;;) {
        // printf(">>");
        // ok = scanf("%s", buf);
        // if(ok == 1) {
            usleep(100);
            msg_len = strlen(buf);
            send_to(sockfd, dst.addr, dst.port, buf, msg_len+1);
        // }
    }

    return 0;
}
