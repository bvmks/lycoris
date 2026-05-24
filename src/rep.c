#include <stdio.h>
#include <stdarg.h>

#include "log.h"
#include "rep.h"

void report_to_log_cb(void *ud, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    log_msg_vl(*(int*)ud, 1, fmt, args);
    va_end(args);
}

void report_to_stream_cb(void *ud, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf((FILE*)ud, fmt, args);
    fputc('\n', (FILE*)ud);
    va_end(args);
}
