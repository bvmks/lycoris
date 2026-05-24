#include <stdlib.h>

#include "strargv.h"


static int is_space(int c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}


static int automaton_pass(const char *src,
                          char **target,  /* NULL on the first pass */
                          char *targstr,  /* -- .. -- */
                          int *wcnt, int *charcnt)
{
    enum automaton_state {
        in_space, unquoted, single_quoted, double_quoted, escaped, dq_esc
    } state;
    int curword, curpos;
    const char *p;

    state = in_space;
    curword = 0;
    curpos = 0;
    for(p = src; *p; p++) {
        char c = *p;
        switch(state) { /* within this switch, we use break to save another
                           char to the target, and we use continue if we
                           don't need to save anything on this step */
        case in_space:
            if(is_space(c))
                continue;
            if(target)
                target[curword] = targstr + curpos;
            curword++;
            if(c == '"') {
                state = double_quoted;
                continue;
            }
            if(c == '\'') {
                state = single_quoted;
                continue;
            }
            if(c == '\\') {
                state = escaped;
                continue;
            }
            state = unquoted;
            break;
        case unquoted:
            if(is_space(c)) {
                state = in_space;
                c = 0;
                break;
            }
            if(c == '"') {
                state = double_quoted;
                continue;
            }
            if(c == '\'') {
                state = single_quoted;
                continue;
            }
            if(c == '\\') {
                state = escaped;
                continue;
            }
            break;
        case single_quoted:
            if(c == '\'') {
                state = unquoted;
                continue;
            }
            break;
        case double_quoted:
            if(c == '\"') {
                state = unquoted;
                continue;
            }
            if(c == '\\') {
                state = dq_esc;
                continue;
            }
            break;
        case escaped:
            state = unquoted;
            break;
        case dq_esc:
            state = double_quoted;
            break;
        }
        /* if we're here, it means we need to store the char */
        if(targstr)
            targstr[curpos] = c;
        curpos++;
    }
    switch(state) {
    case in_space:
        break;
    case unquoted:
        if(targstr)
            targstr[curpos] = 0;
        curpos++;
        break;
    case single_quoted: return mkargv_unmatched_sq;
    case double_quoted: return mkargv_unmatched_dq;
    case escaped:
    case dq_esc:        return mkargv_escaped_eol;
    }
    if(wcnt && charcnt) {
        *wcnt = curword;
        *charcnt = curpos;
    }
    return mkargv_success; 
}

int make_argv_errno = mkargv_success;

char **make_argv(const char *s)
{
    int wc, cc;
    char **res;
    char *str;

    make_argv_errno = automaton_pass(s, NULL, NULL, &wc, &cc);
    if(make_argv_errno != mkargv_success)
        return NULL;

    res = malloc((wc + 2) * sizeof(*res));
    str = malloc(cc + 1);
    res[wc] = NULL;
    res[wc+1] = str;
    automaton_pass(s, res, str, NULL, NULL);
    return res;
}

void dispose_argv(char **p)
{
    int npos = 0;
    while(p[npos])
        npos++;
    free(p[npos+1]);
    free(p);
}

const char *mkargv_diags(int code)
{
    switch(code) {
    case mkargv_success:      return "success";
    case mkargv_unmatched_sq: return "unmatched single quote";
    case mkargv_unmatched_dq: return "unmatched double quote";
    case mkargv_escaped_eol:  return "escaped end of line";
    default:                  return "unknown error";
    }
}

#ifdef STRARGV_TEST

#include <stdio.h>

int main(int argc, char **argv)
{
    char **res;
    int i;

    if(argc < 2) {
        fprintf(stderr, "Please specify the string as the first arg\n");
        return 1;
    }

    printf("{%s}\n", argv[1]);
    res = make_argv(argv[1]);
    if(!res) {
        fprintf(stderr, "ERROR: %s\n", mkargv_diags(make_argv_errno));
        return 1;
    }
    for(i = 0; res[i]; i++)
        printf("[%s]\n", res[i]);
    dispose_argv(res);
    printf("... disposed successfully\n");
    return 0;
}

#endif
