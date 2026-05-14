#ifndef _MS_KEYUTILS_H
#define _MS_KEYUTILS_H


void place_timemark(int tm, unsigned char p[4]);

void increment_buf(unsigned char *buf, int len);

int all_zeroes(const unsigned char *buf, int len);

int get_random(void *buf, int len);

void fill_noise(unsigned char *mem, int len);

int rand_from_range(int first, int last);

#endif
