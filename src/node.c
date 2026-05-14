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
#include "ms_nodecfg.h"
#include "ms_peers.h"

#include "ms_sig.h"
#include "ms_lh.h"
#include "hexdata.h"
#include "addrport.h"

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

static void enlist_peer_conf(struct peer_conf** list, struct peer_conf* conf)
{
    conf->next = *list;
    *list = conf;
}

static void add_test_peers(struct ms_node_cfg* cfg)
{
    int res;
    char* node1 = "a473aa3ee7ecfb25daee";
    struct peer_conf* conf1;
    struct peer_conf* conf2;

    conf1 = malloc(sizeof(*conf1));
    conf2 = malloc(sizeof(*conf2));

    res = hexstr2data(conf1->node_id, node_id_size, node1);
    if(res != node_id_size) {
        message(mlv_normal, "invalid `node_id' [%s]\n", node1);
        return;
    }
    str2ip(&conf1->ip, "127.0.0.1");
    conf1->port = def_port;
    enlist_peer_conf(&cfg->first_peer, conf1);

    // res = hexstr2data(conf2->node_id, node_id_size, node1);
    // if(res != node_id_size) {
    //     message(mlv_normal, "invalid `node_id' [%s]\n", node1);
    //     return;
    // }
    // str2ip(&conf2->ip, "127.0.0.1");
    // conf2->port = def_port2;
    // enlist_peer_conf(&cfg->first_peer, conf2);
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
    
    ensure_wdir();
    message_set_verbosity(mlv_debug);

    sue_alloc_init_default();

    sue_sel_init(&selector);

    /*
     * config "hadcoded" just for now...
     * i will make (steal) text parser for cfg loading later (maybe)
    */
    node_cfg = make_node_def_cfg();
    add_test_peers(node_cfg);
    node_cfg->listen_port = use_port;

    receiver = make_udp_receiver(&selector, node_cfg);
    if(!receiver) {
        message(mlv_alert, "[FATAL] failed to construct udp_receiver\n");
        return -1;
    }
    message(mlv_debug, "[DEBUG] udp_receiver initialized\n");

    sigtarget = prepare_sig_handlers(&selector, receiver);
    message(mlv_debug, "[DEBUG] signal handlers initialized\n");

    lhtarget = prepare_loophooks(&selector, receiver);
    message(mlv_debug, "[DEBUG] loophooks initialized\n");

    if(!start_udp_receiver(receiver))
    {
        message(mlv_alert, "[FATAL] failed to start udp_receiver\n");
        return -1;
    }
    message(mlv_debug, "[DEBUG] udp_receiver started\n");


    message(mlv_debug, "[DEBUG] entering main loop...\n");
    res = sue_sel_go(&selector);
    if(res == -1)
        message(mlv_normal, "main loop reported error\n");
    message(mlv_debug, "[DEBUG] exited main loop\n");

    free_targets(sigtarget, lhtarget);
    
    return 0;
}
