#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

#include "log.h"

static int stderr_log_mode = llv_normal | llv_private;

static FILE *log_file = NULL;
static int file_log_mode = llv_disable;

struct extra_log {
    int level;
    extra_log_callback f;
    void *userdata;
    struct extra_log *next;
};

struct extra_log *first_extra = NULL;

void setup_stderr_log(int mode)
{
    stderr_log_mode = mode;
}

int setup_file_log(int mode, const char *filename)
{
    if(log_file) {
        fclose(log_file);
        log_file = NULL;
        file_log_mode = llv_disable;
    }
    if(mode == llv_disable)
        return 1;
    log_file = fopen(filename, "a");
    if(!log_file) {
        log_perror(llv_alert, "setup_file_log", filename);
        return 0;
    }
    file_log_mode = mode;
    return 1;
}

static int should_log(int channel_mode, int message_mode)
{
    return channel_mode &&
        (channel_mode & 0xff) >= (message_mode & 0xff) &&
        (!(message_mode & llv_private) || (channel_mode & llv_private));
}

static char* lvl2a(int level) 
{
    switch (level & 0xff) {
    case llv_alert:  return "[ALERT]";
    case llv_normal: return "[INFO] ";
    case llv_info:   return "[INFO] ";
    case llv_debug:  return "[DEBUG]";
    case llv_debug2: return "[DEBUG]";
    default:         return "[     ]";
    }
}

static const char * const month_names[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

enum {log_head_padding = 8};

static int compose_msg_head(char* buf, int level, int bald)
{
    time_t tt;
    struct tm *gmt;

    if(bald){
        memset(buf, ' ', log_head_padding);
        return log_head_padding;
    }
    else {
        tt = time(NULL);
        gmt = gmtime(&tt);
        return sprintf(buf, "%s [%02d:%02d:%02d %02d-%3.3s-%d %ld] ",
                       lvl2a(level),
                       gmt->tm_hour, gmt->tm_min, gmt->tm_sec,
                       gmt->tm_mday, month_names[gmt->tm_mon],
                       gmt->tm_year + 1900,
                       (long)getpid());
    }
}


void log_msg_vl(int level, int bald, const char *fmt, va_list args)
{
    static char buf[4096];
    char *message;
    int len, bufrest, mlen;
    int do_stderr, do_file;
    struct extra_log *tmp;

    do_stderr = should_log(stderr_log_mode, level);
    do_file   = should_log(file_log_mode, level);

    if(!do_stderr && !do_file && !first_extra)
        return;

    len = compose_msg_head(buf, level, bald);
    message = buf + len;
    bufrest = sizeof(buf) - len;

    mlen = vsnprintf(message, bufrest, fmt, args);
    if(mlen >= bufrest)
        mlen = bufrest - 1;
    len += mlen;

    buf[len] = '\n';
    len++;
    buf[len] = 0;

    if(do_stderr) {
        fputs(buf, stderr);
        fflush(stderr);
    }
    if(do_file) {
        fputs(buf, log_file);
        fflush(log_file);
    }
    for(tmp = first_extra; tmp; tmp = tmp->next)
        if(should_log(tmp->level, level))
            tmp->f(tmp->userdata, buf);
}


void log_msg(int level, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    log_msg_vl(level, 0, fmt, args);
    va_end(args);
}

void log_msg_bald(int level, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    log_msg_vl(level, 1, fmt, args);
    va_end(args);
}

void log_perror(int level, const char *s1, const char *s2)
{
    if(s1 && *s1)
        log_msg(level, "%s: %s: %s", s1, s2, strerror(errno));
    else
        log_msg(level, "%s: %s", s2, strerror(errno));
}

struct extra_log* setup_extra_log(extra_log_callback cb, void *userdata, int level)
{
    struct extra_log *tmp;
    tmp = malloc(sizeof(*tmp));
    tmp->level = level;
    tmp->f = cb;
    tmp->userdata = userdata;
    tmp->next = first_extra;
    first_extra = tmp;
    return tmp;
}

int change_extra_log(struct extra_log* el, int level)
{
    struct extra_log *tmp;
    for(tmp = first_extra; tmp; tmp = tmp->next)
        if(tmp == el) {
            tmp->level = level;
            return 1;
        }
    return 0;
}

int remove_extra_log(struct extra_log* el)
{
    struct extra_log **p;
    struct extra_log *tmp;
    p = &first_extra;
    while(*p) {
        if(*p == el) {
            tmp = *p;
            *p = tmp->next;
            free(tmp);
            return 1;
        }
        p = &(*p)->next;
    }
    return 0;
}

