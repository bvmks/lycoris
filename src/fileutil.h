#ifndef _FILEUTIL_H
#define _FILEUTIL_H


void settle_localpath(char **target, const char *localpath);

char *concat_path(const char *dir, const char *fname);
void dispose_path(char *p);

int file_is_there(const char *path);
int dir_is_there(const char *path);

int make_directory_path(const char *path, int skip_the_last);

#endif
