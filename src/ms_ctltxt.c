#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <sue/sue_base.h>

#include "_version.h"
#include "log.h"
#include "ms_ctl.h"
#include "strargv.h"


static const char text_help[] =
    "The following commands are recognized:\n"
    "    help [<command>]  show help text\n"
    "    log <level>       enable/disable log messages (see ''help log'')\n"
    "    show <what>       show various information (see ''help show'')\n"
    "    shutdown          shut the node down\n"
    "    exit/quit         close the session\n"
    "Type ''help <command>'' to get help on a specific command.\n"
    ;

static const char text_help_help[] =
    "The ''help'' command shows help texts;\n"
    "Type ''help <command>'' to get help on a specific command.\n"
    ;

static const char text_help_exit_quit[] =
    "Both ''exit'' and ''quit'' just closes your session\n"
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
    "The ''stat'' command shows information about specific peer.\n"
    "Peer may be specified by ip:port or node_id or configured_name\n"
    "The result may be one of following:\n"
    "    unknown                we don't know such peer, at all\n"
    "    known:<descritpion>    peer are known so will show it's info\n"
    ;

static const char text_help_show[] =
    "The ''show'' shows information depending on its subcommand.\n"
    "The subcommand must be one of following:\n"
    "    conf                 dump the node configuration\n"
    "    id                   show the node's identity information\n"
    "    report               show the peer report (the same info as the\n"
    "                         written to the logs upon SIGUSR1)\n"
    "    version              show the daemon's version and the build date\n"
    ;


static void con_ses_handle_help(struct ms_ctl_session *ses, char **argv)
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
        fputs(text_help_show, ses->stream);
    else
    if(0 == strcmp(argv[1], "log"))
        fputs(text_help_log, ses->stream);
    else
        fprintf(ses->stream, "* Sorry, nothing is known about ``%s''\n",
                             argv[1]);
}

static void con_ses_handle_show(struct ms_ctl_session *ses, char **argv)
{

}

static void con_ses_log_cb(void *userdata, const char *message)
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
                fputs("Error removing the log channel (bug)\n", ses->stream);
                log_msg(llv_alert, "bug in set_log_level (1)");
            }
        } else {
            fputs("Log messages are already off\n", ses->stream);
        }
    } else {
        if(ses->log) {
            r = change_extra_log(ses->log, level | llv_private);
            if(r) {
                fputs("OK\n", ses->stream);
            } else {
                fputs("Log channel not found (bug)\n", ses->stream);
                log_msg(llv_alert, "bug in set_log_level (2)");
            }
        } else {
            ses->log =
                setup_extra_log(&con_ses_log_cb, ses, level | llv_private);
        }
    }
}

static void con_ses_handle_log(struct ms_ctl_session *ses, char **argv)
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
        if(0 == strcmp(argv[1], names[idx].name)) {  /* found! */
            set_log_level(ses, names[idx].level);
            return;
        }
    fprintf(ses->stream, "Level ``%s'' unknown, try ``help log''\n", argv[1]);
}

static void con_ses_handle_shutdown(struct ms_ctl_session *ses, char **argv)
{
    if(argv[1] && 0 == strcmp(argv[1], "really")) {
        fputs("Breaking the main loop, bye-bye\n", ses->stream);
        log_msg(llv_normal,
            "Shutting down on command from the control console");
        sue_sel_break(ses->master->the_selector);
    } else {
        fputs("* Please give me the word ``really'' to confirm\n",
              ses->stream);
    }
}

static void send_commit(struct ms_ctl_session *ses)
{
    if(!ses->to_close)
        fputs("== ", ses->stream);
    fflush(ses->stream);
}

static void
con_ses_handle_command(struct ms_ctl_session *ses, const char *cmd, int len)
{
    char **argv;
    argv = make_argv(cmd);
    if(!argv) {
        fprintf(ses->stream, "* PARSE ERROR (%s)\n",
                             mkargv_diags(make_argv_errno));
        send_commit(ses);
        return;
    }
    if(!argv[0]) {
        /* actually nothing to do */
    } else
    if(0 == strcmp(argv[0], "exit") || 0 == strcmp(argv[0], "quit")) {
        fputs("Bye.\n", ses->stream);
        ses->to_close = 1;
    } else
    if(0 == strcmp(argv[0], "help")) {
        con_ses_handle_help(ses, argv);
    } else
    if(0 == strcmp(argv[0], "show")) {
        con_ses_handle_show(ses, argv);
    } else
    if(0 == strcmp(argv[0], "shutdown")) {
        con_ses_handle_shutdown(ses, argv);
    } else
    if(0 == strcmp(argv[0], "log")) {
        con_ses_handle_log(ses, argv);
    } else
    {
        fprintf(ses->stream, "* ERROR command (%s) unknown, try ``help''\n",
                             argv[0]);
    }

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

static void con_data_fd_handler(struct sue_fd_handler *h, int r, int w, int x)
{
    struct feda_con_session *ses;
    int res;

    if(!r || w || x) {
        servlog_message(llv_alert,
            "con_data_fd_handler: unexpected combination %d %d %d", r, w, x);
        return;
    }

    ses = h->userdata;
    res = read(h->fd, ses->buf, sizeof(ses->buf) - ses->buf_used);
    if(res <= 0) {
        if(res == -1)
            servlog_perror(llv_normal, "con_data_fd_handler", "read");
        destroy_session(ses);
        return;
    }

    ses->buf_used += res;

    while(ses->buf_used > 0) {
        int pos = find_eol(ses->buf, ses->buf_used);
        int rest;
        if(pos == -1) {
            if(ses->buf_used > sizeof(ses->buf) - 5) {
                static const char errmsg[] =
                    "** LINE TOO LONG **\n"
                    "Skipping; send a EOL char to recover\n";
                write(h->fd, errmsg, sizeof(errmsg)-1);
                ses->skipping = 1;
                ses->buf_used = 0;
            }
            break;
        }
        ses->buf[pos] = 0;
        if(!ses->skipping)
            con_ses_handle_command(ses, ses->buf, pos);
        rest = ses->buf_used - pos - 1;
        if(rest > 0)
            memmove(ses->buf, ses->buf + pos + 1, rest);
        ses->buf_used -= (pos + 1);
        ses->skipping = 0;   /* we've just seen EOL, anyway! */
    }
    if(ses->to_close)
        destroy_session(ses);
}

static void listen_fd_handler(struct sue_fd_handler *h, int r, int w, int x)
{
    struct feda_console *fc;
    struct feda_con_session *ses;
    int fd;

    if(!r || w || x) {
        servlog_message(llv_alert,
            "listen_fd_handler: unexpected combination %d %d %d", r, w, x);
        return;
    }

    fc = h->userdata;

    fd = accept(h->fd, NULL, NULL);
    if(fd == -1) {
        servlog_perror(llv_alert, NULL, "accept");
        return;
    }

    ses = malloc(sizeof(*ses));
    ses->master = fc;
    ses->fdh.fd = fd;
    ses->fdh.want_read = 1;
    ses->fdh.want_write = 0;
    ses->fdh.want_except = 0;
    ses->fdh.userdata = ses;
    ses->fdh.handle_fd_event = &con_data_fd_handler;
    ses->skipping = 0;
    ses->to_close = 0;
    ses->buf_used = 0;
    ses->next = fc->first;
    ses->log_id = NULL;
    ses->stream = fdopen(fd, "w");
    fc->first = ses;

    sue_sel_register_fd(fc->the_selector, &ses->fdh);
    send_commit(ses);  /* to display the prompt */
}





static int
set_socket_name(struct sockaddr_un *sun, struct server_conf_info *conf)
{
    int len;

    settle_ctlsockpath(conf);
    if(!conf->control_socket_path || !*conf->control_socket_path) {
        servlog_message(llv_alert, "don't have the control socket path");
        return 0;
    }
    len = strlen(conf->control_socket_path);
    if(len + 1 > sizeof(sun->sun_path)) {
        servlog_message(llv_alert, "control socket path is too long");
        return 0;
    }
    strcpy(sun->sun_path, conf->control_socket_path);
    return 1;
}

static int cleanup_old_socket(const char *path)
{
    int r;
    struct stat sb;

    r = stat(path, &sb);
    if(r == -1) {
        if(errno == ENOENT) /* no file, nothing to do, everything's fine */
            return 1;
        servlog_perror(llv_alert, "stat", path);
        return 0;
    }
    if((sb.st_mode & S_IFMT) != S_IFSOCK) {
        servlog_message(llv_alert, "%s exists and is not a socket", path);
        return 0;
    }
    r = unlink(path);
    if(r == -1) {
        servlog_perror(llv_alert, "unlink", path);
        return 0;
    }
    return 1;
}

struct feda_console *
launch_control_console(struct sue_event_selector *sel,
                       struct server_conf_info *conf,
                       struct feda_udp_receiver *feda_rx)
{
    struct feda_console *fc;
    int sd, r, save_umask;
    struct sockaddr_un sun;

    sd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(!sd) {
        servlog_perror(llv_alert, "launch_control_console", "socket");
        return NULL;
    }
    sun.sun_family = AF_UNIX;
    r = set_socket_name(&sun, conf);
    if(!r) {
        /* diagnostic is printed already */
        close(sd);
        return NULL;
    }
    r = cleanup_old_socket(sun.sun_path);
    if(!r) {
        servlog_message(llv_alert,
            "socket cleanup failed, will run with no control socket");
        close(sd);
        return NULL;
    }
    save_umask = umask(077);
    r = bind(sd, (struct sockaddr*)&sun, sizeof(sun));
    if(r == -1) {
        servlog_perror(llv_alert, "launch_control_console", sun.sun_path);
        close(sd);
        umask(save_umask);
        return NULL;
    }
    umask(save_umask);
    listen(sd, 5);

    fc = malloc(sizeof(*fc));
    fc->fdh.fd = sd;
    fc->fdh.want_read = 1;
    fc->fdh.want_write = 0;
    fc->fdh.want_except = 0;
    fc->fdh.userdata = fc;
    fc->fdh.handle_fd_event = &listen_fd_handler;
    fc->the_selector = sel;
    fc->the_config = conf;
    fc->the_feda_rx = feda_rx;
    fc->path = strdup(sun.sun_path);
    fc->first = NULL;
    sue_sel_register_fd(sel, &fc->fdh);

    return fc;
}

void destroy_control_console(struct feda_console *con)
{
    int r;

    while(con->first)
        destroy_session(con->first);

    sue_sel_remove_fd(con->the_selector, &con->fdh);
    close(con->fdh.fd);

    r = unlink(con->path);
    if(r == -1)
        servlog_perror(llv_normal, "warning: unlink", con->path);

    free(con->path);
    free(con);
}
