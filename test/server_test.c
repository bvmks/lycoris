#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include "../src/ms_proto.h"
#include "../src/socks.h"
#include "../src/addrport.h"

enum {
    port = 24880
};


int main_loop(int sock)
{
    int loop, ok, result, chk, got_peer;
    struct ms_received_packet recvd_packet;
    struct ms_connection connection;
    loop = 1;
    got_peer = 0;
    
    fprintf(stderr, "[DEBUG] Enterring main loop\n");
    while(loop) {
        ok = ms_recv_packet(sock, &recvd_packet);

        if(ok == -1) {
            perror("ms_recv_packet");
            loop = 0;
            result = -1;
        }
        printf("Received from: %s\n", addrport2a(&recvd_packet.src_addr));
        if(!got_peer) {
            printf("new peer\n");
            connection_init(&connection, &recvd_packet.src_addr, recvd_packet.packet.header.s_id);
            got_peer = 1;
        }
        printf("type: %s\n", ms_type2a(recvd_packet.packet.header.type));
        if (recvd_packet.packet.header.type.type == mst_post) {
            printf("data: %s\n", recvd_packet.packet.data);
        }
        else {
            printf("data:\n");
        }

        if(recvd_packet.packet.header.type.type != mst_ctrl) {
            chk = list_mask_check(&connection.session.recvd, &recvd_packet.packet);
            if(chk == 0) {
                printf("is a new packet\n");
                list_push(&connection.session.recvd, &recvd_packet.packet);
            }
            else if(chk == 1) {
                printf("is a duplicate\n");
            }
            if (chk != -1) {
                printf("confirmation sent\n");
                ok = ms_send_confirm(sock, &connection, recvd_packet.packet.header.seq);
                if(ok == -1) {
                    perror("ms_send_ctrl");
                    loop = 0;
                    result = -1;
                }
            }
        }

        
        
    }
    fprintf(stderr, "[DEBUG] Quitting main loop\n");
    return result;
}

int main(int argc, char** argv)
{
    int main_sockfd;
    struct sockaddr_in addr;
    socklen_t addr_len;
    srand(time(NULL));
    main_sockfd = make_sock(SOCK_DGRAM, INADDR_ANY, port);
    if(main_sockfd == -1) {
        perror("make_sock");
        fprintf(stderr, "[DEBUG] Failed to init main socket\n");
    }
    getsockname(main_sockfd, (struct sockaddr*)&addr, &addr_len);
    printf("bound to: %s:%d\n", inet_ntoa(addr.sin_addr), port);
    main_loop(main_sockfd);
    return 0;
}


