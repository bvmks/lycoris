/* 
  This program listens to you no matter what bullshit you tell, 
  until you get tired and sleepy. If you keep silence for 3 seconds,
  the program assumes you went to sleep and it is no longer needed, 
  so it terminates.
*/
#include <stdio.h>
#include <unistd.h>
#include <sue/sue_base.h>

#ifndef SLEEP_INTERVAL
#define SLEEP_INTERVAL 3
#endif


struct perfect_listener {
    struct sue_event_selector *the_selector;
    struct sue_timeout_handler to_h;
    struct sue_fd_handler fd_h;
};

static void perfect_listener_fdh(struct sue_fd_handler *h, int r, int w, int x)
{
    char buf[1024]; 
    int rc;
    struct perfect_listener *the_pfl = (struct perfect_listener*)(h->userdata);
    if(!r) {
        /* this should never happen in reality; anyway, it would be a
           bad idea to read when the stream isn't ready */
        return;
    }
    rc = read(0, buf, sizeof(buf));
    if(rc == -1) {
        perror("stdin");
        sue_sel_break(the_pfl->the_selector);
        return;
    } 
    if(rc == 0) {
        printf("EOF, darling?  Ok, bye-bye :-)\n");
        sue_sel_break(the_pfl->the_selector);
        return;
    }

    sue_sel_remove_timeout(the_pfl->the_selector, &the_pfl->to_h);
    sue_timeout_set_from_now(&the_pfl->to_h, SLEEP_INTERVAL, 0);
    sue_sel_register_timeout(the_pfl->the_selector, &the_pfl->to_h);
}

static void perfect_listener_toh(struct sue_timeout_handler *h)
{
    struct perfect_listener *the_pfl = (struct perfect_listener*)(h->userdata);
    sue_sel_break(the_pfl->the_selector);
    printf("Sleep, baby, sleep...\n");
}

static void perfect_listener_init(struct perfect_listener *pl,
                                  struct sue_event_selector *sel)
{
    pl->the_selector = sel;

    pl->to_h.userdata = pl;
    pl->to_h.handle_timeout = &perfect_listener_toh;
    sue_timeout_set_from_now(&pl->to_h, SLEEP_INTERVAL, 0);
    sue_sel_register_timeout(sel, &pl->to_h);

    pl->fd_h.fd = 0; /* stdin */
    pl->fd_h.want_read = 1;
    pl->fd_h.want_write = 0;
    pl->fd_h.want_except = 0;
    pl->fd_h.userdata = pl;
    pl->fd_h.handle_fd_event = &perfect_listener_fdh;
    sue_sel_register_fd(sel, &pl->fd_h);
}

int main() 
{
    int res;
    struct sue_event_selector selector;
    struct perfect_listener pfl;

    sue_alloc_init_default();
    sue_sel_init(&selector);

    perfect_listener_init(&pfl, &selector);

    printf("Entering the main loop... \n");
    res = sue_sel_go(&selector);
    if(res == -1) {
        perror("sue_sel_go");
    }
    printf("Main loop exited\n");

    return 0;
}

