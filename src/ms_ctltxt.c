#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "_version.h"
#include "addrport.h"
#include "log.h"
#include "ms_ctl.h"
#include "ms_peers.h"
#include "strargv.h"
#include "ms_nodecfg.h"
#include "ms_rx.h"
#include "utils.h"

#include "ms_ctl.h"
#include "ms_ctlpar.h"


static const char text_help[] =
    "The following commands are recognized:\n"
    "    help [<command>]       show help text\n"
    "    log <level>            enable/disable log messages (see ''help log'')\n"
    "    bind <iport>           binds this session to specified inner port (see ''help bind'')\n"
    "    show <what>            show various information (see ''help show'')\n"
    "    send <where> <what>    sends specified thing to specified peer (see ''help send'')\n"
    "    shutdown               shut the node down\n"
    "    exit/quit              close the session\n"
    "Type ''help <command>'' to get help on a specific command.\n"
    ;

static const char text_help_help[] =
    "The ''help'' command shows help texts;\n"
    "Type ''help <command>'' to get help on a specific command.\n"
    ;

static const char text_help_exit_quit[] =
    "Both ''exit'' and ''quit'' commands just closes your session\n"
    ;

static const char text_help_log[] =
    "The ''log'' command sets the level of the log messages you will get.\n"
    "The level must be one of following:\n"
    "    no/off/disable         turn log messages off completely\n"
    "    q/quiet                only see the alert level messages\n"
    "    yes/on/enable/normal   set level to ``normal''\n"
    "    verb/verbose/info      see alert, normal and info messages\n"
    "    debug                  see everything up to the debug level\n"
    "    debug2                 see REALLY everything\n"
    ;

static const char text_help_shutdown[] =
    "The ''shutdown'' command causes the node to shut down.\n"
    "Please specify the word ''really'' as the only parameter to confirm you\n"
    "really want it.  Otherwise, the command will tell you it needs the\n"
    "word to be specified.\n"
    ;

static const char text_help_stat[] =
    "The ''stat'' command shows information about specified peer.\n"
    "Peer may be specified by ip:port or by node_id or by configured_name\n"
    "The result may be one of following:\n"
    "    unknown                we don't know such peer\n"
    "    known: <descritpion>   peer are known so will show it's info\n"
    ;

static const char text_help_show[] =
    "The ''show'' command shows information depending on its subcommand.\n"
    "The subcommand must be one of following:\n"
    "    conf                 dump the node configuration\n"
    "    report               show the peer report (the same info as the\n"
    "                         written to the logs upon SIGUSR1)\n"
    "    version              show the daemon's version and the build date\n"
    ;

static const char text_help_bind[] =
    "The ''bind'' command binds you to node's inner port.\n"
    "The port must be in range  0 < iport < 16 \n"
    "To perform ''send'' or ''recv'' operations you must\n"
    "be bound to inner port.\n"
    "Only one control session can be bound to one iport.\n"
    ;

static const char text_help_send[] =
    "The ''send'' command send specified ''thing''\n"
    "to specified peer and specified iport\n"
    "send <peer> <iport> <what> <what_desc>\n"
    "\n"
    "The ''thing'' may be one of following:\n"
    "    msg <text>           just text msg\n"
    "    file <path>          send file\n"
    "\n"
    "Peer can be specified with ip:port/node_id/name\n"
    "And destination iport must follow it\n"
    ;

static const char text_help_recv[] =
    "Bebra\n"
    ;


static void handle_help(struct ms_ctl_session *ses, char **argv)
{
    if(!argv[1])
        fputs(text_help, ses->stream);
    else
    if(0 == strcmp(argv[1], "help"))
        fputs(text_help_help, ses->stream);
    else
    if(0 == strcmp(argv[1], "exit") || 0 == strcmp(argv[1], "quit"))
        fputs(text_help_exit_quit, ses->stream);
    else
    if(0 == strcmp(argv[1], "shutdown"))
        fputs(text_help_shutdown, ses->stream);
    else
    if(0 == strcmp(argv[1], "show"))
        fputs(text_help_show, ses->stream);
    else
    if(0 == strcmp(argv[1], "stat"))
        fputs(text_help_stat, ses->stream);
    else
    if(0 == strcmp(argv[1], "bind"))
        fputs(text_help_bind, ses->stream);
    else
    if(0 == strcmp(argv[1], "send"))
        fputs(text_help_send, ses->stream);
    else
    if(0 == strcmp(argv[1], "recv"))
        fputs(text_help_recv, ses->stream);
    else
    if(0 == strcmp(argv[1], "log"))
        fputs(text_help_log, ses->stream);
    else
        fprintf(ses->stream, "* ERROR unknown help command option ''%s''\n",
                             argv[1]);
}

static void txt_handle_show(struct ms_ctl_session *ses, char **argv)
{
    if(!argv[1]) {
        fputs(text_help_show, ses->stream);
        return;
    }
    if(0 == strcmp(argv[1], "conf")) {
        dump_configuration_to_stream(ses->master->the_cfg, ses->stream);
    } else
    if(0 == strcmp(argv[1], "report")) {
        udp_receiver_streamrep(ses->master->the_rx, ses->stream);
    } else
    if(0 == strcmp(argv[1], "version")) {
        fputs("MS node vers. " MS_VERSION
              " (compiled " __DATE__ ")\n",
              ses->stream);
    } else
    {
        fprintf(ses->stream, "* ERROR unknown show command option ''%s''\n",
                argv[1]);
    }

}


static void txt_handle_bind(struct ms_ctl_session *ses, char **argv)
{
    long long iport;
    if(!argv[1]) {
        fputs(text_help_bind, ses->stream);
        return;
    }
    if(!str2integer(argv[1], &iport)) {
        fputs("* ERROR bind command needs integer argument\n", ses->stream);
        return;
    }
    if(ctl_handle_bind(ses, iport)) {
        fputs("Bound\n", ses->stream);
        return;
    }

    switch (ms_ctl_errno) {
    case ctl_err_bind_already_bound:
        fputs("* ERROR session already bound\n", ses->stream);
        break;
    case ctl_err_bind_port_occupied:
        fputs("* ERROR iport occupied\n", ses->stream);
        break;
    case ctl_err_invalid_arg:
        fputs("* ERROR invalid iport\n", ses->stream);
        break;
    }
}

static void txt_handle_send(struct ms_ctl_session *ses, char **argv)
{
    struct ms_udp_receiver* rx = ses->master->the_rx;
    struct ms_peer* peer;
    long long dst_iport;
    unsigned int ip;
    unsigned short port;
    int res;

    if(!argv[1]) {
        fputs(text_help_send, ses->stream);
        return;
    }

    if(*argv[1] == '@') {
        res = ctl_resolve_peer_from_name(rx->the_crx, argv[1] + 1, &ip, &port);
        if(!res) {
            fputs("* ERROR unknown peer\n", ses->stream);
            return;
        }
    } else
    if(!str2ipport(&ip, &port, argv[1])) {
        fputs("* ERROR invalid peer address\n", ses->stream);
        return;
    }

    peer = get_peer_record(rx->peers , ip, port, 0);
    if(!peer) {
        fputs("* ERROR unknown peer\n", ses->stream);
        return;
    }

    if(!argv[2]) {
        fputs("* ERROR destination iport must be specified\n", ses->stream);
        return;
    }

    if(!str2integer(argv[2], &dst_iport)) {
        fputs("* ERROR destination iport must be integer\n", ses->stream);
        return;
    }

    if(!argv[3]) {
        fputs("* ERROR sending target must be specified\n", ses->stream);
        return;
    }


    if(0 == strcmp(argv[3], "msg")) {
        if(!argv[4]) {
            fputs("* ERROR messages must be specified \n", ses->stream);
            return;
        }
        res = ctl_handle_send(ses, peer, dst_iport, argv[4], strlen(argv[4]));
    }else
    if(0 == strcmp(argv[3], "file")) {
        fputs("* ERROR files sending not supported yet\n", ses->stream);
        return;
    }else {
        fprintf(ses->stream, "* ERROR unknown sending target ''%s''\n", argv[3]);
        return;
    }

    if(res)
        return;

    switch (ms_ctl_errno) {
    case ctl_err_send_invalid_iport:
        fputs("* ERROR invalid destination iport\n", ses->stream);
        break;
    case ctl_err_send_msg_too_long:
        fputs("* ERROR message too long\n", ses->stream);
        break;
    case ctl_err_send_not_bound:
        fputs("* ERROR session must be bound to send\n", ses->stream);
        break;
    case ctl_err_send_no_assoc:
        fprintf(ses->stream,"* ERROR no association with %s\n", peer_description(peer));
        break;
    }
}

static void txt_handle_recv(struct ms_ctl_session *ses, char **argv)
{
    struct received_post* msg;
    if(ses->rxq_len == 0) {
        fputs("nothing to receive\n", ses->stream);
        return;
    }
    else {
        msg = ses->rxq_first;
        ses->rxq_first = ses->rxq_first->next;
        if(!ses->rxq_first)
            ses->rxq_last = NULL;
    }
    fprintf(ses->stream, "msg src/dst %d/%d payload:\n%s\n",
            msg->src_iport, msg->dst_iport, msg->payload);
    free(msg->payload);
    free(msg);
}

static void txt_log_cb(void *userdata, const char *message)
{
    struct ms_ctl_session *ses = userdata;
    fputs(message, ses->stream);
    fflush(ses->stream);
}

static void set_log_level(struct ms_ctl_session* ses, int level)
{
    int r;
    if(level == llv_disable) {
        if(ses->log) {
            r = remove_extra_log(ses->log);
            ses->log = NULL;
            if(r) {
                fputs("Log messages disabled\n", ses->stream);
            } else {
                fputs("* ERROR error removing the log channel (bug)\n", ses->stream);
                log_msg(llv_alert, "bug in set_log_level (1)");
            }
        } else {
            fputs("Log messages are already off\n", ses->stream);
        }
    } else {
        if(ses->log) {
            r = change_extra_log(ses->log, level | llv_private);
            if(r) {
                fputs("Log level changed\n", ses->stream);
            } else {
                fputs("* ERROR log channel not found (bug)\n", ses->stream);
                log_msg(llv_alert, "bug in set_log_level (2)");
            }
        } else {
            ses->log =
                setup_extra_log(&txt_log_cb, ses, level | llv_private);
        }
    }
}

static void txt_handle_log(struct ms_ctl_session *ses, char **argv)
{
    struct levit {
        const char *name;
        int level;
    };
    static const struct levit names[] = {
        { "no",      llv_disable },
        { "off",     llv_disable },
        { "disable", llv_disable },
        { "alert",   llv_alert   },
        { "quiet",   llv_alert   },
        { "q",       llv_alert   },
        { "yes",     llv_normal  },
        { "on",      llv_normal  },
        { "normal",  llv_normal  },
        { "enable",  llv_normal  },
        { "info",    llv_info    },
        { "verb",    llv_info    },
        { "verbose", llv_info    },
        { "debug",   llv_debug   },
        { "debug2",  llv_debug2  },
        { NULL,      0            }
    };

    int idx;

    if(!argv[1]) {
        fputs(text_help_log, ses->stream);
        return;
    }
    for(idx = 0; names[idx].name; idx++)
        if(0 == strcmp(argv[1], names[idx].name)) {
            set_log_level(ses, names[idx].level);
            return;
        }
    fprintf(ses->stream, "* ERROR unknown log level ''%s'', try ''help log''\n",
            argv[1]);
}

static void txt_handle_shutdown(struct ms_ctl_session *ses, char **argv)
{
    if(argv[1] && 0 == strcmp(argv[1], "really")) {
        fputs("Breaking the main loop, bye-bye\n", ses->stream);
        ctl_handle_shutdown(ses);
    } else {
        fputs("Please give me the word ''really'' to confirm\n",
              ses->stream);
    }
}

void send_commit(struct ms_ctl_session *ses)
{
    if(!ses->to_close)
        fputs("== ", ses->stream);
    fflush(ses->stream);
}

static void
txt_handle_command(struct ms_ctl_session *ses, const char *cmd, int len)
{
    char **argv;
    argv = make_argv(cmd);
    if(!argv) {
        fprintf(ses->stream, "* ERROR parsing failed (%s)\n",
                             mkargv_diags(make_argv_errno));
        send_commit(ses);
        return;
    }
    if(!argv[0]) {
    } else
    if(0 == strcmp(argv[0], "exit") || 0 == strcmp(argv[0], "quit")) {
        fputs("Bye.\n", ses->stream);
        ses->to_close = 1;
    } else
    if(0 == strcmp(argv[0], "help")) {
        handle_help(ses, argv);
    } else
    if(0 == strcmp(argv[0], "show")) {
        txt_handle_show(ses, argv);
    } else
    if(0 == strcmp(argv[0], "bind")) {
        txt_handle_bind(ses, argv);
    } else
    if(0 == strcmp(argv[0], "send")) {
        txt_handle_send(ses, argv);
    } else
    if(0 == strcmp(argv[0], "recv")) {
        txt_handle_recv(ses, argv);
    } else
    if(0 == strcmp(argv[0], "shutdown")) {
        txt_handle_shutdown(ses, argv);
    } else
    if(0 == strcmp(argv[0], "log")) {
        txt_handle_log(ses, argv);
    } 
    else {
        fprintf(ses->stream, "* ERROR unknown command ''%s'', try ''help''\n",
                argv[0]);
    }

    fflush(ses->stream);
    dispose_argv(argv);
    send_commit(ses);
}

static int find_eol(const char *buf, int buflen)
{
    int i;
    for(i = 0; i < buflen; i++)
        if(buf[i] == '\n')
            return i;
    return -1;
}

int txt_read(struct ms_cparser* cp, unsigned int* used)
{
    struct ms_ctl_session *ses = cp->the_session;
    unsigned int u = 0;
    int parsing_res = cpres_want_more;
    char* current_line_ptr;

    while (cp->buf_rd_pos < cp->buf_filled) {
        unsigned int available = cp->buf_filled - cp->buf_rd_pos;
        int pos = find_eol((char*)cp->buf + cp->buf_rd_pos, available);

        if (pos == -1) {
            if (cp->buf_filled >= sizeof(cp->buf) - 1) {
                static const char errmsg[] =
                    "* ERROR line is too long\n"
                    "Skipping; send a EOL char to recover\n";
                write(ses->fdh.fd, errmsg, sizeof(errmsg) - 1);
                
                cp->txt_skipping = 1;
                cp->buf_filled = 0;
                cp->buf_rd_pos = 0;
                u = 0;
            }
            break;
        }

        current_line_ptr = (char*)cp->buf + cp->buf_rd_pos;
        cp->buf_rd_pos += pos + 1;
        current_line_ptr[pos] = '\0';

        if (!cp->txt_skipping)
            txt_handle_command(ses, current_line_ptr, pos);
        cp->txt_skipping = 0;
        u = cp->buf_rd_pos;
    }

    *used = u;
    return parsing_res;
}
