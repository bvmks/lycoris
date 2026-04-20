#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#include "../src/net/ms_proto.h"

#include "../src/fileutil.h"
#include "../src/message.h"
#include "../src/hexdata.h"

enum {
    def_port = 24880
};


static int main_loop(struct ms_node* node)
{
    int loop, result;
    loop = 0;
    result = 0;
    
    fprintf(stderr, "[DEBUG] Enterring main loop\n");
    while(loop) {
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
    start_node(node);
    
    return main_loop(node);
}


