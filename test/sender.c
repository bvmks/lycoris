#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../src/net/ms_proto.h"
#include "../src/net/socks.h"

enum {
    port = 24880
};

int main(int argc, char **argv)
{
    int sockfd, peer_id, msg_len, ok;
    struct addrport dst;
    struct ms_connection peer;
    char buf[1000];
    // char buf[] =  "bebra";

    if (argc < 2) {
        printf("usage: %s ip:port\n", argv[0]);
        return 1;
    }
    
    str2addrport(&dst, argv[1]);
    sockfd = make_sock(SOCK_DGRAM, INADDR_ANY, port);
    peer_id = generate_id(dst.addr);
    connection_init(&peer, &dst, peer_id);

    for(;;) {
        printf(">>");
        ok = scanf("%s", buf);
        if(ok == 1) {
            // usleep(1000);
            msg_len = strlen(buf);
            ms_send_post(sockfd, &peer, buf, msg_len+1);
        }
    }

    return 0;
}
