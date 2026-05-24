#ifndef STRARGV_H_SENTRY
#define STRARGV_H_SENTRY

char **make_argv(const char *s);
void dispose_argv(char **p);

enum {
    mkargv_success,
    mkargv_unmatched_sq,
    mkargv_unmatched_dq,
    mkargv_escaped_eol
};

extern int make_argv_errno;
const char *mkargv_diags(int code);

#endif
