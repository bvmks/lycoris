#include <stdlib.h>
#include <signal.h>

#include <sue/sue_base.h>

#include "ms_rx.h"
#include "message.h"

#include "ms_sig.h"

struct ms_signal_target {
    struct sue_signal_handler siginth;
    struct sue_signal_handler sigquith;
    struct sue_signal_handler sigtermh;
    struct sue_signal_handler sigkillh;
    struct ms_udp_receiver *rx;
    struct sue_event_selector *the_selector;
};

static void quit_on_sig(struct sue_signal_handler *h, int cnt)
{
    struct ms_signal_target *t = h->userdata;
    message(mlv_debug2, "quit_on_sig called\n");
    sue_sel_break(t->the_selector); 
}

struct ms_signal_target* prepare_sig_handlers(struct sue_event_selector* s,
                                              struct ms_udp_receiver* rx)
{
    struct ms_signal_target *t;
    t = malloc(sizeof(*t));


    t->siginth.signo = SIGKILL;
    t->siginth.userdata = t;
    t->siginth.handle_signal = quit_on_sig;

    t->siginth.signo = SIGINT;
    t->siginth.userdata = t;
    t->siginth.handle_signal = quit_on_sig;

    t->sigquith.signo = SIGQUIT;
    t->sigquith.userdata = t;
    t->sigquith.handle_signal = quit_on_sig;

    t->sigtermh.signo = SIGTERM;
    t->sigtermh.userdata = t;
    t->sigtermh.handle_signal = quit_on_sig;

    t->rx = rx;
    t->the_selector = s;

    sue_sel_register_signal(s, &t->siginth);
    sue_sel_register_signal(s, &t->sigquith);
    sue_sel_register_signal(s, &t->sigtermh);

    return t;
}
