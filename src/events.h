#ifndef _MS_EVENTS_H
#define _MS_EVENTS_H

#include <signal.h>

struct signal_handler {
    int signo;

    void *userdata;

    void (*handle_signal)(struct signal_handler*, int signo);
};

struct sig_handler_item {
    struct signal_handler* h;
    struct sig_handler_item* next;
};


struct event_selector {
    int fd;
    struct sig_handler_item *signalhandlers;
    int breakflag;
};

void sel_init(struct event_selector *s);

void sel_done(struct event_selector *s);

void sel_set_fd(struct event_selector *s, int fd)
{
    s->fd = fd;
}

void sel_register_signal(struct event_selector *s, struct signal_handler *h);
void sel_remove_signal(struct event_selector *s, struct signal_handler *h);

void handle_signal_events(const struct sig_handler_item *list);

void enter_signal_section(const struct sig_handler_item *list, sigset_t *saved_mask);

void leave_signal_section(const sigset_t *saved_mask);


#endif // !_MS_EVENTS_H
