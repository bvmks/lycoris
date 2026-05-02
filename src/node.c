#include <stdlib.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/select.h>

#include <sue/sue_base.h>
#include <sue/sue_aloc.h>


#include "fileutil.h"
#include "message.h"
#include "ms_rx.h"

#include "ms_sig.h"
#include "ms_lh.h"

enum {
    def_port = 24880,
};

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

void free_targets(struct ms_signal_target* st, struct ms_loophook_target* lt) {
    if(st)
        free(st);
    if(lt)
        free(lt);
}

int main(int argc, char** argv)
{
    struct sue_event_selector selector;
    struct ms_udp_receiver* receiver;
    struct ms_node_cfg* node_cfg;
    struct ms_signal_target* sigtarget;
    struct ms_loophook_target* lhtarget;
    int res;
    
    ensure_wdir();
    message_set_verbosity(mlv_debug2);

    sue_alloc_init_default();

    sue_sel_init(&selector);
    message(mlv_debug, "[DEBUG] event selector initialized...\n");


    /*
     * config "hadcoded" just for now...
     * i will make (steal) text parser for cfg loading later (maybe)
    */
    node_cfg = make_node_def_cfg();
    message(mlv_debug, "[DEBUG] config initialized...\n");

    receiver = make_udp_receiver(&selector, node_cfg);
    if(!receiver) {
        message(mlv_alert, "[FATAL] Failed to make node, quitting...\n");
        return -1;
    }
    message(mlv_debug, "[DEBUG] node made...\n");

    sigtarget = prepare_sig_handlers(&selector, receiver);
    message(mlv_debug, "[DEBUG] signal handlers initialized...\n");

    lhtarget = prepare_loophooks(&selector, receiver);
    message(mlv_debug, "[DEBUG] loophooks initialized...\n");

    if(!start_udp_receiver(receiver))
    {
        message(mlv_alert, "[FATAL] Failed to start node, quitting...\n");
        return -1;
    }
    message(mlv_normal, "Started node\n");


    message(mlv_debug, "[DEBUG] Entering main loop\n");
    res = sue_sel_go(&selector);
    if(res == -1) {
        message(mlv_alert, "Main loop returned error\n");
    }
    message(mlv_debug, "[DEBUG] Quitting main loop\n");

    free_targets(sigtarget, lhtarget);
    
    return 0;
}


