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
#include "log.h"
#include "ms_rx.h"
#include "ms_nodecfg.h"
#include "ms_peers.h"

#include "ms_sig.h"
#include "ms_lh.h"
#include "hexdata.h"
#include "ms_rx.h"

#include "_version.h"

enum {
    def_port = 24880,
    def_port2 = 24881,
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

static void free_targets(struct ms_signal_target* st, struct ms_loophook_target* lt) {
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
    unsigned short use_port;

    if(argc > 1)
        use_port = def_port2;
    else
        use_port = def_port;

    setup_stderr_log(llv_debug | llv_private);

    log_msg(llv_alert, "starting ms node " MS_VERSION);

    sue_alloc_init_default();
    sue_sel_init(&selector);

    /*
     * config "hadcoded" just for now...
     * i will make (steal) text parser for cfg loading later (maybe)
    */
    ensure_wdir();
    node_cfg = make_node_def_cfg();
    node_cfg->listen_port = use_port;

    receiver = make_udp_receiver(&selector, node_cfg);
    if(!receiver) {
        log_msg(llv_alert, "failed to construct udp_receiver");
        return -1;
    }

    log_msg(llv_alert, "udp_receiver initialized successfully");
    log_msg_bald(llv_alert, "our id:     %s", 
                 hexdata2a(receiver->comctx.identity->node_id, node_id_size));
    log_msg_bald(llv_alert, "our pubkey: %s",
                 hexdata2a(receiver->comctx.identity->master_public_key, public_key_size));


    sigtarget = prepare_sig_handlers(&selector, receiver);
    log_msg(llv_debug, "signal handlers initialized");

    lhtarget = prepare_loophooks(&selector, receiver);
    log_msg(llv_debug, "loophooks initialized");

    if(!start_udp_receiver(receiver))
    {
        log_msg(llv_alert, "failed to start udp_receiver");
        return -1;
    }
    log_msg(llv_alert, "udp_receiver started successfully");


    log_msg(llv_alert, "entering main loop");
    res = sue_sel_go(&selector);
    if(res == -1)
        log_msg(llv_alert, "main loop reported error");
    log_msg(llv_alert, "exited main loop");

    free_targets(sigtarget, lhtarget);
    
    return 0;
}
