#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include "../src/ms_headers.h"
#include "../src/ms_communicate.h"
#include "../src/ms_peer.h"
#include "../src/ms_session.h"
#include "../src/socks.h"

enum {
    buf_len = 508,
    max_msg_len = buf_len - sizeof(struct ms_header),
    port = 24880
};

int main(int argc, char** argv)
{
    int sockfd;

    srand(time(NULL));

    sockfd = make_sock(SOCK_DGRAM, INADDR_ANY, port);
}


