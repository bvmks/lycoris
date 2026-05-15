#ifndef MS_LOG_H
#define MS_LOG_H

#include <stdarg.h>

enum log_levels {
    llv_disable =  0,
    llv_alert   =  1,   /* show always                     */
    llv_normal  =  2,    /* normal messages                 */
    llv_info    =  3,    /* show additional info            */
    llv_debug   =  4,    /* show debug messages             */
    llv_debug2  =  5,    
                           

    llv_private = 256    
};

void log_msg_vl(int level, int bald, const char *fmt, va_list args);

/* log msg with 'head' it's: lvl + date/time + msg */
void log_msg(int level, const char *fmt, ...);
/* only msg with padding*/
void log_msg_bald(int level, const char *fmt, ...);

    /* s1 may be NULL or empty */
void log_perror(int level, const char *s1, const char *s2);

void setup_stderr_log(int mode);

    /* returns boolean */
int setup_file_log(int mode, const char *filename);

typedef void (*extra_log_callback)(void *, const char *);
struct extra_log;

struct extra_log* setup_extra_log(extra_log_callback cb, void *usrdata, int level);
int change_extra_log(struct extra_log* el, int level);
int remove_extra_log(struct extra_log* el);

#endif
