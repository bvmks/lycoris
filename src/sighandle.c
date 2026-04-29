#include "sighandle.h"
#include <signal.h>

#ifndef _NSIG
#define _NSIG 64
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif


static int sighdl_count[_NSIG];  /* currenly active handlers per signal */
static struct sigaction saved_sigactions[_NSIG];
static volatile sig_atomic_t signal_counters[_NSIG];

static void the_handler(int signo)
{
    if(signo < 1 || signo > _NSIG)
        return;
    signal_counters[signo-1]++;
}

void sue_signals_init()
{
}

void signals_add(int signo)
{
    if(signo < 1 || signo > _NSIG)
        return;
    if(0 >= sighdl_count[signo-1]++) {
        /* set up the handler! */
        struct sigaction act;
        act.sa_handler = &the_handler;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        sigaction(signo, &act, saved_sigactions + (signo-1));
    }
}

void signals_remove(int signo)
{
    if(signo < 1 || signo > _NSIG)
        return;
    if(--sighdl_count[signo-1] == 0) {
        /* remove the handler, restore the saved disposition */
        sigaction(signo, saved_sigactions + (signo-1), NULL);
    }
}

int signals_get_counter(int signo)
{
    return signal_counters[signo-1];
}

void signals_zero_counters()
{
    int i;
    for(i = 0; i < _NSIG; i++)
        signal_counters[i] = 0;
}

struct signal_handler {
    int signo;

    void *userdata;

    void (*handle_signal)(struct signal_handler*, int signo);
};

struct sel_signal_item {
    struct signal_handler* h;
    struct sel_signal_item* next;
};

void handle_signal_events(const struct sel_signal_item *list)
{
    const struct sel_signal_item *p = list;
    while(p) {
        const struct sel_signal_item *nx = p->next;
        int cnt = signals_get_counter(p->h->signo);
        if(cnt > 0)
            p->h->handle_signal(p->h, cnt);
        p = nx;
    }
    signals_zero_counters();
}

