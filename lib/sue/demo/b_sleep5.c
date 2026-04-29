/* This simple demo program just sleeps 5 seconds and exits 
   using SUE base mechanism of timeout events 
*/
#include <stdio.h>
#include <sue/sue_base.h>

#ifndef SLEEP_INTERVAL
#define SLEEP_INTERVAL 5
#endif


/* the userdata field is used as a pointer to the selector object */
static void the_timeout_handler(struct sue_timeout_handler *hdl)
{
    printf("Hook called... \n");
    sue_sel_break((struct sue_event_selector*) (hdl->userdata));
}

int main() 
{
    int res;
    struct sue_event_selector selector;
    struct sue_timeout_handler atm;

    sue_alloc_init_default();
    sue_sel_init(&selector);

    atm.userdata = &selector;
    atm.handle_timeout = &the_timeout_handler;
    sue_timeout_set_from_now(&atm, SLEEP_INTERVAL, 0);

    sue_sel_register_timeout(&selector, &atm);

    printf("Entering the main loop... \n");
    res = sue_sel_go(&selector);
    if(res == -1) {
        perror("sue_sel_go");
    }
    printf("Main loop exited\n");

    return 0;
}

