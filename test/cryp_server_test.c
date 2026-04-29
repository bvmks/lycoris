#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/select.h>

#include "../src/net/ms_proto.h"

#include "../src/fileutil.h"
#include "../src/message.h"
#include "../src/hexdata.h"
#include "../src/events.h"
#include "../src/net/ms_rx.h"

enum {
    def_port = 24880,
    select_timeout_usec = 1000000,
};

struct signal_target {
    struct signal_handler siginth;
};

static void calculate_timeout(struct ms_node* n, struct timespec* tm) 
{
    /* 
     * for now it's stupid const value
     * must be calculated depending on peers timeouts
     * will make later (at least believe so)
     */
    tm->tv_sec = 0;
    tm->tv_nsec = select_timeout_usec;
}

static void prepare_sig_handlers(struct sig_handler_item** list) {
    struct signal_handler h;
    register_signal(list,&h);
}

static int main_loop(struct ms_node* node)
{
    int res = 0, result = 0, needbreak = 0;
    struct sig_handler_item* sig_handlers = NULL;
    prepare_sig_handlers(&sig_handlers);

    fprintf(stderr, "[DEBUG] Enterring main loop\n");
    while(!needbreak) {
        fd_set rd_fds;
        sigset_t ps_mask;
        struct timespec rd_timeout;

        FD_ZERO(&rd_fds);
        FD_SET(node->fd, &rd_fds);

        enter_signal_section(sig_handlers, &ps_mask);
        handle_signal_events(sig_handlers);

        calculate_timeout(node, &rd_timeout);
        res = pselect(node->fd + 1, &rd_fds, NULL, NULL, &rd_timeout, &ps_mask);
        if(res == -1 && errno != EINTR)
            return -1;

        if(res == -1 && errno == EINTR)
            handle_signal_events(sig_handlers);
        leave_signal_section(&ps_mask);

        if(res > 0) {
            do_recv(node);
        }

    }
    fprintf(stderr, "[DEBUG] Quitting main loop\n");
    return result;
}

static int ensure_wdir()
{
    char* wdir = NULL;
    char* keysdir = NULL;
    char* configfile = NULL;
    int res = 0;

    settle_localpath(&wdir, ".ms");
    keysdir = concat_path(wdir, "keys");
    configfile = concat_path(wdir, "node.config");

    res = make_directory_path(keysdir, 0);

    free(wdir);
    free(keysdir);
    free(configfile);
    return res;
}

int main(int argc, char** argv)
{
    struct event_selector selector;
    struct ms_node* node;
    struct ms_node_cfg* node_cfg;
    
    ensure_wdir();
    

    node = make_node();
    /*
     * it's "hadcoded" just for now...
     * i will make (steal) text parser for cfg loading later (maybe)
    */
    node_cfg = make_node_def_cfg();
    set_node_cfg(node, node_cfg);

    if(load_node_id(node)) {
        message(mlv_normal, "Fatal error occured, quitting...\n");
        return 1;
    }

    message(mlv_normal, "Started node: %s\n", hexdata2a(node->id->node_id, node_id_size));

    sel_init(&selector);

    start_node(node);
    
    return main_loop(node);
}


