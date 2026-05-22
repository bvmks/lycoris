#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "ms_ctl.h"
#include "ms_nodecfg.h"
#include "log.h"
#include "ms_rx.h"
#include "ms_ctlhdl.h"

enum {
    con_sess_buff_size = 1024,
    con_sess_max_direct = 4096,
};

enum {
    ms_conn_mode_binary = 0,
    ms_conn_mode_text = 1,

};

const char* ses_description(const struct ms_ctl_session* ses)
{
    static char buf[128];
    char* p = buf;
    p += sprintf(p, "CTL SES %d", ses->id);
    if(ses->bound) {
        *p = ' ';
        p++;
        p += sprintf(p, "[%d]", ses->iport);
    }
    return buf;
}

static void dispose_session(struct ms_ctl_session *ses)
{
    struct ms_control_receiver *crx = ses->master;
    struct sue_event_selector *sel = crx->the_selector;
    struct ms_ctl_session **p;

    sue_sel_remove_fd(sel, &ses->fdh);
    if(ses->log) {
        remove_extra_log(ses->log);
        ses->log = NULL;
    }

    if(ses->bound)
        crx->ports[ses->iport] = NULL;

    if(ses->cur_cmd)
        dispose_cmd(ses->cur_cmd);
    ms_cparser_cleanup(&ses->parser);

    p = &crx->first;
    while(*p) {
        if(*p == ses) {
            *p = (*p)->next;
            free(ses);
            return;
        }
        p = &(*p)->next;
    }
}

static void close_session(struct ms_ctl_session *ses)
{
    if(ses->stream)
        fclose(ses->stream);
    else
        close(ses->fdh.fd);
    dispose_session(ses);
}


static int set_socket_name(struct sockaddr_un* addr, struct ms_node_cfg* cfg) 
{
    int len;
    if(!cfg->control_sock_path || !*cfg->control_sock_path) {
        log_msg(llv_alert, "don't have the control socket path");
        return 0;
    }
    len = strlen(cfg->control_sock_path);
    if(len + 1 > sizeof(addr->sun_path)) {
        log_msg(llv_alert, "control socket path is too long");
        return 0;
    }
    strcpy(addr->sun_path, cfg->control_sock_path);
    return 1;
}

static int check_clean_socket(const char *path)
{
    int r;
    struct stat sb;

    r = stat(path, &sb);
    if(r == -1) {
        if(errno == ENOENT)
            return 1;
        log_perror(llv_alert, "stat", path);
        return 0;
    }
    if((sb.st_mode & S_IFMT) != S_IFSOCK) {
        log_msg(llv_alert, "%s exists and is not a socket", path);
        return 0;
    }
    r = unlink(path);
    if(r == -1) {
        log_perror(llv_alert, "unlink", path);
        return 0;
    }
    return 1;
}

static void control_fd_handler(struct sue_fd_handler *h, int r, int w, int x)
{
    struct ms_ctl_session* ses = h->userdata;
    struct ms_cparser* parser = &ses->parser;
    struct ms_ccmd_result* cmd_result;
    int res;

    log_msg(llv_debug2, 
            "%s: control_fd_handler called (%s)(%s)", 
            ses_description(ses), r ? "r" : "-" , w ? "w" : "-");

    if(!r || w || x) {
        log_msg(llv_alert,
                "%s: control_fd_handler: unexpected combination %d %d %d",
                ses_description(ses), r, w, x);
        return;
    }

    res = ms_ctlparser_read(parser, h->fd);
    switch (res) {
    case ctlparser_res_want_more:
        break;
    case ctlparser_res_error:
        log_msg(llv_debug, 
                "%s: control command parsing failed",
                ses_description(ses));
    case ctlparser_res_finished:
        cmd_result = process_ccmd(parser, parser->target);
        send_response(ses, cmd_result);
        dispose_cmd_res(cmd_result);
        ms_cparser_reset(parser);
        break;

    case ctlparser_res_fatal:
        log_msg(llv_alert, 
                "%s: control command parsing fatal error, closing connection",
                ses_description(ses));
        close_session(ses);
        return;
    case ctlparser_res_conn_closed:
        log_msg(llv_debug, 
                "%s: connection terminated",
                ses_description(ses));
        dispose_session(ses);
        return;
    default:
        log_msg(llv_alert, 
                "%s: unknown parsing result (%d) (bug)", 
                ses_description(ses), res);
        return;
    }

    if(ses->to_close)
        close_session(ses);
}

static void send_intro(struct ms_ctl_session* ses)
{
    /*TODO: of course it shoud look into our config, but for now so*/
    fputs("HELLO\nAUTH NO\nMODE TEXT\n\n", ses->stream);
    fflush(ses->stream);
}

static void listen_fd_handler(struct sue_fd_handler *h, int r, int w, int x)
{
    struct ms_control_receiver *crx;
    struct ms_ctl_session *ses;
    int fd;

    if(!r || w || x) {
        log_msg(llv_alert,
            "listen_fd_handler: unexpected combination %d %d %d (BUG)", r, w, x);
        return;
    }

    crx = h->userdata;

    fd = accept(h->fd, NULL, NULL);
    if(fd == -1) {
        log_perror(llv_alert, "listen_fd_handler", "accept");
        return;
    }


    ses = malloc(sizeof(*ses));
    ses->master = crx;
    ses->fdh.fd = fd;
    ses->fdh.want_read = 1;
    ses->fdh.want_write = 0;
    ses->fdh.want_except = 0;
    ses->fdh.userdata = ses;
    ses->fdh.handle_fd_event = &control_fd_handler;
    ses->to_close = 0;
    ses->log = NULL;
    ses->stream = fdopen(fd, "w");

    ses->iport = ms_conn_iport_undef;
    ses->bound = 0;

    ses->id = crx->ses_id_counter;
    crx->ses_id_counter++;
    
    ses->next = crx->first;
    crx->first = ses;

    ses->cur_cmd = make_cmd();
    ms_cparser_init(&ses->parser, ses->cur_cmd, ses, ctlparser_m_text);

    log_msg(llv_debug, "NEW %s", ses_description(ses));

    sue_sel_register_fd(crx->the_selector, &ses->fdh);
    send_intro(ses);
}

struct ms_control_receiver *
launch_control_receiver(struct sue_event_selector *sel,
                        struct ms_node_cfg *cfg,
                        struct ms_udp_receiver *rx)
{
    struct ms_control_receiver *crx;
    struct sockaddr_un addr;
    int sock, r, save_umask;

    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if(sock == -1) {
        log_perror(llv_alert, "launch_control_receiver", "socket");
        return NULL;
    }
    addr.sun_family = AF_UNIX;

    r = set_socket_name(&addr, cfg);
    if(!r) {
        close(sock);
        return NULL;
    }
    r = check_clean_socket(addr.sun_path);
    if(!r) {
        log_msg(llv_alert,
            "socket cleanup failed, will run with no control socket");
        close(sock);
        return NULL;
    }
    save_umask = umask(077);
    r = bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    if(r == -1) {
        log_perror(llv_alert, "launch_control_receiver", addr.sun_path);
        close(sock);
        umask(save_umask);
        return NULL;
    }
    umask(save_umask);
    listen(sock, 5);

    crx = malloc(sizeof(*crx));

    crx->fdh.fd = sock;
    crx->fdh.want_read = 1;
    crx->fdh.want_write = 0;
    crx->fdh.want_except = 0;
    crx->fdh.userdata = crx;
    crx->fdh.handle_fd_event = &listen_fd_handler;

    crx->ses_id_counter = 0;

    crx->the_selector = sel;
    crx->the_cfg = cfg;
    crx->the_rx = rx;
    crx->path = strdup(addr.sun_path);

    set_control_receiver(rx, crx);

    memset(crx->ports, 0 , sizeof(crx->ports));

    fill_cmd_trie();

    sue_sel_register_fd(sel, &crx->fdh);

    return crx;
}


void dispose_control_receiver(struct ms_control_receiver *crx)
{
    struct ms_ctl_session* p = crx->first;

    while(p) {
        struct ms_ctl_session* tmp = p;
        p = p->next;
        close_session(tmp);
    }
    crx->first = NULL;
    
    clear_cmd_trie();

    sue_sel_remove_fd(crx->the_selector, &crx->fdh);
    close(crx->fdh.fd);
    if(crx->path)
        free(crx->path);
}
