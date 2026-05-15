#ifndef _REPORT_H
#define _REPORT_H

typedef void (*report_callback)(void *, const char *, ...);

    /* userdata must be int*, pointing to the logging level value */
void report_to_log_cb(void *userdata, const char *fmt, ...);

#endif
