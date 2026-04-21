#include "ms_rx.h"
#include "addrport.h"
#include "../message.h"
#include <arpa/inet.h>

int send_to(int fd, unsigned int ip, unsigned short port,
             const void *buf, int len)
{
    struct sockaddr_in saddr;
    int r;

    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(ip);
    saddr.sin_port = htons(port);

    r = sendto(fd, buf, len, 0, (struct sockaddr*)&saddr, sizeof(saddr));
    if(r < 1) {
        message_perror(mlv_alert, "ALERT", "sendto");
        message(mlv_alert, "error sending %d bytes to %s",
                                    len, ipport2a(ip, port));
        return -1;
    } else if(r != len) {
        message(mlv_alert,
                "dgram len mismatch: %d to send, %d sent", len, r);
        return -1;
    } else {
        message(mlv_debug2, "[fd==%d] sent %d bytes to %s",
                                     fd, len, ipport2a(ip, port));
    }
    return 0;
}
