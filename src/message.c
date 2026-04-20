#include <stdio.h>
#include <stdarg.h>

#include "message.h"


static int msg_verbosity = 0;

void message_set_verbosity(int verbosity)
{
    msg_verbosity = verbosity;
}

void message(int level, const char *fmt, ...)
{
    va_list args;
    if(level > msg_verbosity)
        return;

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

void message_perror(int level, const char *s1, const char *s2)
{
    if(level > msg_verbosity)
        return;
    if(s1) {
        fputs(s1, stderr);
        fputs(": ", stderr);
    }
    perror(s2);
}
