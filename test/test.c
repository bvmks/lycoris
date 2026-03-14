#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <endian.h>


int main(int argc, char** argv)
{
    unsigned short a = 103;
    unsigned short b = 105;
    short c = (a - b);
    printf("%d\n",c);
}


