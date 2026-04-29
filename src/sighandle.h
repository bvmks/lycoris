#ifndef _MS_SIGHANDLER_H
#define _MS_SIGHANDLER_H

void sue_signals_init();
void signals_add(int signo);
void signals_remove(int signo);
int signals_get_counter(int signo);
void signals_zero_counters();


#endif
