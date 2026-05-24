#include <stdlib.h>

#include <sue/sue_base.h>
#include <sue/sue_aloc.h>


#include "stupid_peers_parser.h"
#include "fileutil.h"
#include "log.h"
#include "ms_rx.h"
#include "ms_nodecfg.h"

#include "ms_sig.h"
#include "ms_lh.h"
#include "ms_ctl.h"

#include "_version.h"

enum {
    def_port = 24880,
    def_port2 = 24881,
};

struct cmdline_args {
    char* config_file;
    int log_stderr;
    int log_stderr_only;
    int verbosity;
    int help;
    char errmsg[80];


    int test_p;
};

static void set_def_args(struct cmdline_args* args) 
{
    args->config_file = NULL;
    args->log_stderr = 0;
    args->log_stderr_only = 0;
    args->verbosity = llv_normal;
    args->help = 0;
    args->errmsg[0] = 0;


    args->test_p = 0;
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

static void free_targets(struct ms_signal_target* st, struct ms_loophook_target* lt) 
{
    if(st)
        free(st);
    if(lt)
        free(lt);
}

static void print_help()
{
    fputs(
        "msnode\n"
        "vers. " MS_VERSION "\n"
        "\n"
        "usage: \"msnode <options>\"\n"
        "options:\n"
        "    -c <config_file> defaul is $HOME./ms/node.conf\n"
        "    -e               send the log messages to stderr as well\n"
        "    -E               send the log messages to stderr _only_\n"
        "    -q               go quiet   (verbosity=1)\n"
        "    -v               go verbose (verbosity++)\n"
        "                     may stack like -vv or -v -v\n"
        "    -p               only for test, sets port2\n"
        "    -h               show this text\n"
        "\n",
        stdout);
}

static void print_need_param(char c, struct cmdline_args *args)
{
    sprintf(args->errmsg, "option -%c needs a parameter\n", c);
}

static int
parse_cmdline(int argc, char **argv, struct cmdline_args *args)
{
    int idx = 1;
    int pos;
    while(idx < argc) {
        if(argv[idx][0] == '-') {
            switch(argv[idx][1]) {
            case 'c':
                if(idx+1 >= argc || argv[idx+1][0] == '-') {
                    print_need_param('c', args);
                    return 0;
                }
                args->config_file = (char*)argv[idx+1];
                idx += 2;
                break;
            case 'v':
                pos = 1;
                do {
                    if(args->verbosity < llv_debug2)
                        args->verbosity++;
                    pos++;
                } while(argv[idx][pos] == 'v');
                idx++;
                break;
            case 'e':
                args->log_stderr = 1;
                idx++;
                break;
            case 'q':
                args->verbosity = llv_alert;
                idx++;
                break;
            case 'E':
                args->log_stderr_only = 1;
                idx++;
                break;
            case 'p':
                args->test_p = 1;
                idx++;
                break;
            case 'h':
                args->help = 1;
                idx++;
                break;
            default:
                sprintf(args->errmsg, "unknown option '-%c'\n", argv[idx][1]);
                return 0;
            }
        } else {
            sprintf(args->errmsg, "unknown option '%s'\n", argv[idx]);
            return 0;
        }
    }
    return 1;
}

static void settle_config_path(struct cmdline_args *args)
{
    settle_localpath(&args->config_file, ".ms/node.conf");
}

static void
process_cmdline(int argc, char **argv, struct cmdline_args *args)
{
    int res;

    set_def_args(args);
    res = parse_cmdline(argc, argv, args);
    if(!res) {
        log_msg(llv_alert, "FATAL: wrong args: %s", args->errmsg);
        exit(1);
    }
    if(args->help) {
        print_help();
        exit(0);
    }

    if(args->log_stderr || args->log_stderr_only) {
        setup_stderr_log(args->verbosity | llv_private);
    }

    if(!args->config_file)
        settle_config_path(args);
}

int main(int argc, char** argv)
{
    struct cmdline_args args;
    struct sue_event_selector selector;
    struct ms_udp_receiver* receiver = NULL;
    struct ms_node_cfg* node_cfg = NULL;
    struct ms_signal_target* sigtarget = NULL;
    struct ms_loophook_target* lhtarget = NULL;
    struct ms_control_receiver* ctl_receiver = NULL;
    int res;

    setup_stderr_log(llv_debug);

    process_cmdline(argc, argv, &args);


    log_msg(llv_alert, 
            "Starting ms node vers. " MS_VERSION 
            " (compiled " __DATE__ ")");

    sue_alloc_init_default();
    sue_sel_init(&selector);

    /*
     * config "hadcoded" just for now...
     * i will make (steal) text parser for cfg loading later (maybe)
    */
    ensure_wdir();
    node_cfg = make_node_cfg();
    res = read_node_cfg_file(node_cfg, args.config_file);
    if(!res) {
        log_msg(llv_alert, 
                "failed to load config file (%s)",
                args.config_file);
        return 1;
    }

    settle_keys_dir(node_cfg);
    settle_kndb_dir(node_cfg);
    settle_peerscfg_path(node_cfg);
    node_cfg->has_control_sock = 1;
    settle_ctlsock_path(node_cfg);
    node_cfg->first_peer = parse_peers_file(node_cfg->peers_cfg_file);

    if(args.test_p)
        node_cfg->listen_port = def_port2;
    else
        node_cfg->listen_port = def_port;

    dump_configuration_to_log(node_cfg, llv_debug);


    receiver = make_udp_receiver(&selector, node_cfg);
    if(!receiver) {
        log_msg(llv_alert, "failed to construct udp_receiver");
        return -1;
    }

    sigtarget = prepare_sig_handlers(&selector, receiver);
    lhtarget = prepare_loophooks(&selector, receiver);

    if(!start_udp_receiver(receiver))
    {
        log_msg(llv_alert, "failed to start udp_receiver");
        return -1;
    }

    log_msg(llv_alert, "udp_receiver started, port: %d", receiver->the_cfg->listen_port);

    if(node_cfg->has_control_sock)
        ctl_receiver = launch_control_receiver(&selector, node_cfg, receiver);

    log_msg(llv_debug, "entering main loop");
    res = sue_sel_go(&selector);
    if(res == -1)
        log_msg(llv_alert, "main loop reported error");
    log_msg(llv_debug, "exited main loop");

    dispose_control_receiver(ctl_receiver);

    free_targets(sigtarget, lhtarget);
    
    return 0;
}
