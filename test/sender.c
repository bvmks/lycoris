#include <stdio.h>
#include "../src/ms_proto.h"
#include "../src/socks.h"

enum {
    port = 24880
};

int main(int argc, char **argv)
{
    int sockfd, peer_id, buf_len;
    struct addrport dst;
    struct ms_connection peer;
    char buf[1000];
    buf_len = sizeof(buf);

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
        buf_len = scanf("%s", buf);
        ms_send(sockfd, &peer, mst_post, 0, buf, buf_len);
    }

    return 0;
}
