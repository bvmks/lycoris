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
#include "../src/net/ms_rx.h"
#include "../src/events.h"

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

static void prepare_sig_handlers(struct event_selector* s) {
    struct signal_handler h;
    sel_register_signal(s, &h);
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

void prepare_fd_handler(struct event_selector* s)
{
    struct fd_handler* h;
    h = malloc(sizeof(*h));
    
}

int main(int argc, char** argv)
{
    struct event_selector* selector;
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

    start_node(node);
    message(mlv_normal, "Started node: %s\n", hexdata2a(node->id->node_id, node_id_size));

    selector = alloc_selector_init();
    prepare_fd_handler(selector);
    prepare_sig_handlers(selector);


    fprintf(stderr, "[DEBUG] Entering main loop\n");
    sel_go(selector);
    fprintf(stderr, "[DEBUG] Quitting main loop\n");
    
    return 0;
}


