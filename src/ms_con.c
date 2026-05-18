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

#include <sue/sue_base.h>

#include "ms_con.h"
#include "ms_nodecfg.h"
#include "log.h"
#include "ms_rx.h"
#include "ms_cparser.h"
#include "ms_chandl.h"

enum {
    con_sess_buff_size = 1024,
    con_sess_max_direct = 4096,
};

enum {
    ms_conn_mode_binary = 0,
    ms_conn_mode_text = 1,

    /* inner port numeration starts with 1*/
    ms_conn_iport_undef = -1,
    ms_conn_iport_all = 0,

    ms_conn_iport_max = 16,
};

struct ms_con_session {
    struct ms_con_session* next;

    struct ms_control_receiver *the_master;
    struct sue_fd_handler fdh;
    FILE *stream;

    struct extra_log* log;

    int iport;
    char bound;

    char to_close;

    struct ms_ccmd* cur_cmd;
    struct ms_cparser parser;
};


struct ms_control_receiver {
    struct sue_fd_handler fdh;
    struct sue_event_selector *the_selector;

    struct ms_udp_receiver* the_rx;
    struct ms_node_cfg* the_cfg;

    struct ms_con_session* first;
    struct ms_con_session* ports[ms_conn_iport_max];
    char *path;
};

static void close_session(struct ms_con_session *ses)
{
    struct ms_control_receiver *crx = ses->the_master;
    struct sue_event_selector *sel = crx->the_selector;
    struct ms_con_session **p;

    sue_sel_remove_fd(sel, &ses->fdh);
    if(ses->stream)
        fclose(ses->stream);
    else
        close(ses->fdh.fd);

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


static void session_fd_handler(struct sue_fd_handler *h, int r, int w, int x)
{
    struct ms_con_session* ses = h->userdata;
    struct ms_cparser* parser = &ses->parser;
    struct ms_ccmd_result* res;
    void* read_ptr;
    unsigned long long read_size;
    unsigned long long direct;

    long long rest;
    long long parser_readed;
    unsigned char buf[con_sess_buff_size];
    int rd, parser_res;

    if(!r || w || x) {
        log_msg(llv_alert,
            "session_fd_handler: unexpected combination %d %d %d", r, w, x);
        return;
    }

    direct = ms_cparser_want_direct(parser);
    if(direct) {
        read_ptr = ms_cparser_get_direct(parser);
        read_size = direct > con_sess_max_direct ? con_sess_max_direct : direct;
    }
    else {
        read_ptr = buf;
        read_size = sizeof(buf);
    }

    rd = read(ses->fdh.fd, read_ptr, read_size);
    if(rd <= 0) {
        if(rd == -1)
            log_perror(llv_alert, "session_fd_handler", "read");
        close_session(ses);
        return;
    }

    rest = rd;
    while(rest > 0) {
        parser_res = ms_cparser_feed(parser, ses->cur_cmd, 
                                     direct? NULL:read_ptr, rd, &parser_readed,
                                     direct?1:0);
        rest -= parser_readed;
        switch (parser_res) {
        case ms_cp_res_finished:
            res = handle_ccmd(ses->cur_cmd);
            /* here goes response code*/
            dispose_cmd_res(res);
            break;
        case ms_cp_res_want_more:
            break;
        case ms_cp_res_error:
            log_msg(llv_alert, "control command parsing failed");
            break;
        }
    }

    if(ses->to_close)
        close_session(ses);
}

static void send_intro(struct ms_con_session* ses)
{

}

static void listen_fd_handler(struct sue_fd_handler *h, int r, int w, int x)
{
    struct ms_control_receiver *crx;
    struct ms_con_session *ses;
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
        
    log_msg(llv_normal, "new control connection");

    ses = malloc(sizeof(*ses));
    ses->the_master = crx;
    ses->fdh.fd = fd;
    ses->fdh.want_read = 1;
    ses->fdh.want_write = 0;
    ses->fdh.want_except = 0;
    ses->fdh.userdata = ses;
    ses->fdh.handle_fd_event = &session_fd_handler;
    ses->to_close = 0;
    ses->log = NULL;
    ses->stream = fdopen(fd, "w");

    ses->iport = ms_conn_iport_undef;
    ses->bound = 0;
    
    ses->next = crx->first;
    crx->first = ses;

    ms_cparser_init(&ses->parser, ms_cpm_text);
    ses->cur_cmd = make_cmd();

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

    crx->the_selector = sel;
    crx->the_cfg = cfg;
    crx->the_rx = rx;
    crx->path = strdup(addr.sun_path);

    set_control_receiver(rx, crx);

    memset(crx->ports, 0 , sizeof(crx->ports));

    sue_sel_register_fd(sel, &crx->fdh);

    return crx;
}


void dispose_control_receiver(struct ms_control_receiver *crx)
{
    struct ms_con_session* p = crx->first;

    while(p) {
        struct ms_con_session* tmp = p;
        p = p->next;
        close_session(tmp);
    }
    crx->first = NULL;

    sue_sel_remove_fd(crx->the_selector, &crx->fdh);
    close(crx->fdh.fd);
    if(crx->path)
        free(crx->path);
}
