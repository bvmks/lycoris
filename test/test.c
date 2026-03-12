#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include "../src/liblyc.h"
#include <endian.h>


int main(int argc, char** argv)
{
    struct ms_msg_header h;
    h.type = 0;
    h.timestamp = get_timestamp();
    h.hash = 392872678;

    char* buf = malloc(508);
    char* cur = buf + ms_pack_header(&h, buf);

    struct ms_msg_header* tmp = (struct ms_msg_header*)buf;
    int hash = ntohs(tmp+);
    printf("%d\n", hash);
}


