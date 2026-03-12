#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "../src/liblyc.h"


enum {
    buf_len = 508,
    max_msg_len = buf_len - ms_header_len,
    port = 24880
};

int main(int argc, char** argv)
{
    struct ms_msg_header header;
    struct sockaddr_in own_addr, dst_addr;
    socklen_t dst_addr_len = sizeof(dst_addr);
    int sockfd, ok, msg_len;
    char* buf, *fill_ptr;

    if(argc < 3) {
        printf("Usage: %s IP MSG\n", argv[0]);
        return 1;
    }
    
    msg_len = strlen(argv[2])+1;
    if(msg_len > max_msg_len) {
        printf("MSG is too long");
        return 5;
    }

    dst_addr.sin_family = AF_INET;
    dst_addr.sin_port = htons(port);
    ok = inet_aton(argv[1], &dst_addr.sin_addr);
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
    fill_ptr = buf;

    header.hash = 0;
    header.type = MS_MSG;
    header.timestamp = get_timestamp();

    fill_ptr += ms_pack_header(&header, buf);
    memcpy(fill_ptr, argv[2], msg_len);
    
    ok = sendto(sockfd, buf, ms_header_len + msg_len, 0, (struct sockaddr*)&dst_addr, dst_addr_len);
    if(ok != ms_header_len + msg_len) {
        printf("NOT OK!\n");
        return 7;
    }
    else {
        printf("%i\n", ok);
    }
}


