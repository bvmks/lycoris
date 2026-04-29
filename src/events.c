#include <stdio.h>
#include <stdlib.h>

#include "events.h"
#include "sighandle.h"

void sel_init(struct event_selector *s)
{
    s->fd = -1;
    s->signalhandlers = NULL;
    s->breakflag = 0;
}

void sel_done(struct event_selector *s)
{
    while(s->signalhandlers) {
        struct sig_handler_item *tmp = s->signalhandlers;
        free(s->signalhandlers);
        s->signalhandlers = tmp;
    }
}

void sel_register_signal(struct event_selector *s,
                         struct signal_handler *h)
{
    struct sig_handler_item *tmp;
    tmp = malloc(sizeof(*tmp));
    tmp->h = h;
    tmp->next = s->signalhandlers;
    s->signalhandlers = tmp;
    signals_add(h->signo);
}

void sel_remove_signal(struct event_selector *s,
                       struct signal_handler *h)
{
    struct sig_handler_item **p = &(s->signalhandlers);
    while(*p) {
        if((*p)->h == h) {
            struct sig_handler_item *tmp = *p;
            *p = tmp->next;
            signals_remove(tmp->h->signo);
            free(tmp);
            return;
        }
        p = &((*p)->next);
    }
}

