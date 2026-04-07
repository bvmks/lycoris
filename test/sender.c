#include <stdio.h>
#include <string.h>
#include "../src/ms_proto.h"
#include "../src/socks.h"

enum {
    port = 24880
};

int main(int argc, char **argv)
{
    int sockfd, peer_id, msg_len, ok;
    struct addrport dst;
    struct ms_connection peer;
    char buf[1000];

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
            msg_len = strlen(buf);
            ms_send(sockfd, &peer, mst_post, 0, buf, msg_len+1);
        }
    }

    return 0;
}
