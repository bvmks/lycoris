#ifndef MESSAGE_H_SENTRY
#define MESSAGE_H_SENTRY


/* -1 for quiet, 0 for normal, 1, ..., n for verbose */
void message_set_verbosity(int verbosity);

void message(int level, const char *fmt, ...);

/* s1 may be NULL, s2 must be a valid string (it is passed to perror) */
void message_perror(int level, const char *s1, const char *s2);

enum message_levels {
    mlv_alert  = -1,    /* show even in the "quiet" mode   */
    mlv_normal =  0,    /* normal messages */
    mlv_info   =  1,    /* show in the "verbose" mode only */
    mlv_debug  =  2,
    mlv_debug2  = 3
};

#endif
