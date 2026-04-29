#ifndef _MS_EVENTS_H
#define _MS_EVENTS_H

#include <signal.h>

struct signal_handler {
    int signo;
    void *userdata;
    void (*handle_signal)(struct signal_handler*, int signo);
};

struct sel_signal_item {
    struct signal_handler* h;
    struct sel_signal_item* next;
};

struct loop_hook {
    void *userdata;
    void (*loop_hook)(struct loop_hook*);
};

struct sel_loophook_item {
    struct loop_hook* h;
    struct sel_loophook_item* next;
};

struct fd_handler {
    int fd;
    char want_read, want_write, want_except;
    void *userdata;

    void (*handle_fd_event)(struct fd_handler*, int, int, int);
};

struct event_selector {
    int fd;
    struct fd_handler* fdh;
    struct sel_signal_item *signalhandlers;
    struct sel_loophook_item *loophooks;
    int breakflag;
};

struct event_selector* alloc_selector_init();

void sel_init(struct event_selector *s);

void sel_done(struct event_selector *s);


int sel_go(struct event_selector* s);


void sel_register_signal(struct event_selector *s, struct signal_handler *h);
void sel_remove_signal(struct event_selector *s, struct signal_handler *h);

void handle_signal_events(const struct sel_signal_item *list);


void enter_signal_section(const struct sel_signal_item *list, sigset_t *saved_mask);
void leave_signal_section(const sigset_t *saved_mask);


void sel_register_loop_hook(struct event_selector *s, struct loop_hook *h);
void sel_remove_loop_hook(struct event_selector *s, struct loop_hook *h);

void handle_loophooks(struct sel_loophook_item* s);

void sel_register_fd_handler(struct event_selector* s, struct fd_handler* h);
void handle_fd(struct event_selector* s);


#endif // !_MS_EVENTS_H
