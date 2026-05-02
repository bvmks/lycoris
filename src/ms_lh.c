#include <stdlib.h>

#include <sue/sue_base.h>

#include "ms_rx.h"

#include "ms_sig.h"
#include "message.h"

struct ms_loophook_target {
    struct sue_loop_hook mh;
    struct ms_udp_receiver *rx;
    struct sue_event_selector *the_selector;
};

static void main_hook(struct sue_loop_hook *h)
{
    struct ms_udp_receiver *r = h->userdata;
    r->fd_h = r->fd_h; /* to ease compiler warning */
    message(mlv_debug2, "main_hook called\n");
}

struct ms_loophook_target* prepare_loophooks(struct sue_event_selector* s,
                                             struct ms_udp_receiver* rx)
{
    struct ms_loophook_target *t;

    t = malloc(sizeof(*t));

    t->rx = rx;
    t->the_selector = s;

    t->mh.userdata = t;
    t->mh.loop_hook = &main_hook;

    sue_sel_register_loop_hook(s, &t->mh);
    return t;
}
