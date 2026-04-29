/*
   This demo program waits for a signal. Upon SIGINT (Ctrl-C) it 
   prints a message telling you how much times you did this and
   advicing you to press Ctrl-\ instead (to generate SIGQUIT). 

   Having SIGQUIT, it exits.
*/

#include <stdio.h>
#include <signal.h>
#include <sue/sue_base.h>


/* handling functions are called from within the main loop, not from
   the signal handler directly, so it is okay to use whatever functions
   we want
 */

static void sigquit_hdl(struct sue_signal_handler *h, int cnt)
{
    printf("Got SIGQUIT, exiting...\n");
    sue_sel_break((struct sue_event_selector*)(h->userdata));
}

static void sigint_hdl(struct sue_signal_handler *h, int cnt)
{
    printf("You've pressed Ctrl-C %d time(s). Press Ctrl-\\ instead!\n", 
           ++ *(int*)(&(h->userdata)));
}


int main() 
{
    int res;
    struct sue_event_selector selector;
    struct sue_signal_handler sigqh, siginth;

    sue_alloc_init_default();
    sue_sel_init(&selector);

    sigqh.signo = SIGQUIT;
    sigqh.userdata = &selector;
    sigqh.handle_signal = &sigquit_hdl;
    sue_sel_register_signal(&selector, &sigqh);

    siginth.signo = SIGINT;
    siginth.userdata = (void*)0;   /* used as an integer, not pointer */
    siginth.handle_signal = &sigint_hdl;
    sue_sel_register_signal(&selector, &siginth);

    printf("Entering the main loop... \n");
    res = sue_sel_go(&selector);
    if(res == -1) {
        perror("sue_sel_go");
    }
    printf("Main loop exited\n");

    return 0;
}


