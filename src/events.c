#include <errno.h>
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
        struct sel_signal_item *tmp = s->signalhandlers;
        free(s->signalhandlers);
        s->signalhandlers = tmp;
    }
}

void sel_set_fd(struct event_selector *s, int fd)
{
    s->fd = fd;
}

void sel_register_signal(struct event_selector *s,
                         struct signal_handler *h)
{
    struct sel_signal_item *tmp;
    tmp = malloc(sizeof(*tmp));
    tmp->h = h;
    tmp->next = s->signalhandlers;
    s->signalhandlers = tmp;
    signals_add(h->signo);
}

void sel_remove_signal(struct event_selector *s,
                       struct signal_handler *h)
{
    struct sel_signal_item **p = &(s->signalhandlers);
    while(*p) {
        if((*p)->h == h) {
            struct sel_signal_item *tmp = *p;
            *p = tmp->next;
            signals_remove(tmp->h->signo);
            free(tmp);
            return;
        }
        p = &((*p)->next);
    }
}

void enter_signal_section(const struct sel_signal_item *list,
                                 sigset_t *saved_mask)
{
    sigset_t ps_mask;
    sigemptyset(&ps_mask);
    while(list) {
        sigaddset(&ps_mask, list->h->signo);
        list = list->next;
    }
    sigprocmask(SIG_BLOCK, &ps_mask, saved_mask);
}

void leave_signal_section(const sigset_t *saved_mask)
{
    sigprocmask(SIG_SETMASK, saved_mask, NULL);
}


struct event_selector* alloc_selector_init()
{
    struct event_selector* s;
    s = malloc(sizeof(*s));
    s->fd = -1;
    s->breakflag = 0;
    s->signalhandlers = NULL;
    s->loophooks = NULL;
    s->fdh = NULL;
    return s;
}

void handle_loophooks(struct sel_loophook_item *lhs)
{
    struct sel_loophook_item *p;
    p = lhs;
    while(p) {
        struct sel_loophook_item *nx = p->next;
        p->h->loop_hook(p->h);
        p = nx;
    }
}

void sel_register_loop_hook(struct event_selector *s, struct loop_hook *h)
{
    struct sel_loophook_item *tmp;
    tmp = malloc(sizeof(*tmp));
    tmp->h = h;
    tmp->next = s->loophooks;
    s->loophooks = tmp;
}

void sel_remove_loop_hook(struct event_selector *s, struct loop_hook *h)
{
    struct sel_loophook_item **p = &(s->loophooks);
    while(*p) {
        if((*p)->h == h) {
            struct sel_loophook_item *tmp = *p;
            *p = tmp->next;
            free(tmp);
            return;
        }
        p = &((*p)->next);
    }
}

void handle_fd_event(struct event_selector* s)
{
    if(!s->fdh)
        return;
    s->fdh->handle_fd_event(s->fdh, 
                            s->fdh->want_read, 
                            s->fdh->want_write, 
                            s->fdh->want_except);
}



int sel_go(struct event_selector* s)
{
    int res = 0, result = 0, needbreak = 0;

    while(!needbreak) {
        fd_set rd_fds;
        sigset_t ps_mask;
        struct timespec rd_timeout;

        if(s->signalhandlers) {
            enter_signal_section(s->signalhandlers, &ps_mask);
            handle_signal_events(s->signalhandlers);
        }

        FD_ZERO(&rd_fds);
        FD_SET(s->fd, &rd_fds);

        res = pselect(s->fd + 1, &rd_fds, NULL, NULL, NULL, &ps_mask);
        if(res == -1 && errno != EINTR)
            return -1;

        if(s->signalhandlers) {
            if(res == -1 && errno == EINTR)
                handle_signal_events(s->signalhandlers);
            leave_signal_section(&ps_mask);
        }

        if(res > 0)
            handle_fd_event(s);
        handle_loophooks(s->loophooks);
    }
    return result;
}

void sel_register_fd_handler(struct event_selector* s, struct fd_handler* h) 
{
    if(s->fdh)
        free(s->fdh);
    s->fdh = h;
}


