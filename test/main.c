#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "../src/ms_headers.h"
#include "../src/ms_communicate.h"


enum {
    buf_len = 508,
    max_msg_len = buf_len - sizeof(struct ms_header),
    port = 24880
};

int main(int argc, char** argv)
{
    struct ms_header header;
    struct sockaddr_in own_addr;
    struct ipv4_address dst_addr;
    socklen_t dst_addr_len = sizeof(dst_addr);
    int sockfd, ok, msg_len;
    char* buf, *data_ptr;

    if(argc < 3) {
        printf("Usage: %s IP MSG\n", argv[0]);
        return 1;
    }
    
    msg_len = strlen(argv[2])+1;
    if(msg_len > max_msg_len) {
        printf("MSG is too long");
        return 5;
    }

    dst_addr.port = htons(port);
    ok = inet_aton(argv[1], (struct in_addr*)&dst_addr.address);
    if (!ok) {
        perror("IP");
        return 3;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd == -1) {
        perror("socket");
        return 2;
    }
    
    own_addr.sin_family = AF_INET;
    own_addr.sin_port = htons(port);
    own_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    ok = bind(sockfd, (struct sockaddr*)&own_addr, sizeof(own_addr));
    if(ok == -1) {
        perror("bind");
        return 4;
    }

    buf = malloc(buf_len);
    data_ptr = buf + sizeof(struct ms_header);

    memcpy(data_ptr, argv[2], msg_len);
    ok = ms_send(sockfd, &dst_addr, message, argv[2], msg_len);
    
    // ok = sendto(sockfd, buf, ms_header_len + msg_len, 0, (struct sockaddr*)&dst_addr, dst_addr_len);
    if(ok != sizeof(struct ms_header) + msg_len) {
        printf("NOT OK!\n");
        return 7;
    }
    else {
        printf("%i\n", ok);
    }
}


