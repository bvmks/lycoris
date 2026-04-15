#include <fcntl.h>
#include <unistd.h>

#include "ms_keyutils.h"

int get_random(void *buf, int len)
{
    int fd, rc, cnt;
    fd = open("/dev/urandom", O_RDONLY);
    if(fd == -1)
        return 0;
    cnt = 0;
    while(cnt < len) {
        rc = read(fd, ((char*)buf) + cnt, len - cnt);
        if(rc < 1) {
            close(fd);
            return 0;
        }
        cnt += rc;
    }
    close(fd);
    return 1;
}
