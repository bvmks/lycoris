#ifndef _MS_KEYUTILS_H
#define _MS_KEYUTILS_H

int get_random(void *buf, int len);

void fill_noise(unsigned char *mem, int len);

int rand_from_range(int first, int last);

#endif
