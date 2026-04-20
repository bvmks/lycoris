#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>

#include "fileutil.h"


char *concat_path(const char *dir, const char *fname)
{
    int dirlen, namelen;
    char *res;

    dirlen = strlen(dir);
    namelen = strlen(fname);
    res = malloc(dirlen + namelen + 2);
    strcpy(res, dir);
    res[dirlen] = '/';
    strcpy(res + dirlen + 1, fname);
    return res;
}

void dispose_path(char *p)
{
    free(p);
}

int file_is_there(const char *path)
{
    int r;
    struct stat st;
    r = stat(path, &st);
    return r != -1 && S_ISREG(st.st_mode);
}

int dir_is_there(const char *path)
{
    int r;
    struct stat st;
    r = stat(path, &st);
    return r != -1 && S_ISDIR(st.st_mode);
}

int make_directory_path(const char *path, int skip_the_last)
{
    char *pc, *dr;
    int r;

    if(!skip_the_last) {
        struct stat st;
        r = stat(path, &st);
        if(r != -1) {
            if(S_ISDIR(st.st_mode)) {
                return 0;  /* okay */
            } else {
                errno = ENOTDIR;
                return -1;
            }
        }
        if(errno != ENOENT)
            return -1;
    }

    pc = strdup(path);
    dr = strdup(dirname(pc));
    r = (strcmp(".", dr) == 0) ? 0 : make_directory_path(dr, 0);
    free(dr);
    free(pc);
    if(r == -1)
        return -1;

    r = skip_the_last ? 0 : mkdir(path, 0777);
    return r;
}

void settle_localpath(char **target, const char *localpath)
{
    const char *home;
    char *dir;
    int homelen, lplen;

    if(*target)
        return;

    home = getenv("HOME");
    if(!home || !*home) {
        *target = strdup(localpath);
        return;
    }
    homelen = strlen(home);
    lplen = strlen(localpath);
    dir = malloc(homelen + 1 + lplen + 1);
    strcpy(dir, home);
    dir[homelen] = '/';
    strcpy(dir + homelen + 1, localpath);
    *target = dir;
}
