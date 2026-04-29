#include <stdio.h>

#include "../src/message.h"
#include "../src/net/ms_rx.h"


int main(int argc, char *argv[])
{
    unsigned char buf[1024];
    int cmd;
    set_plain_dgram_head(buf, 0x33);
    cmd = get_plain_dgram_cmd(buf);
    printf("cmd: %x\n", cmd);
}
