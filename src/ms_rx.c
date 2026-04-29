#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ms_rx.h"
#include "ms_nodeid.h"
#include "ms_nodecfg.h"
#include "ms_comm_ctx.h"
#include "ms_txq.h"
#include "ms_peer.h"
#include "socks.h"

#include "addrport.h"
#include "message.h"
#include "fileutil.h"
#include "keyutils.h"
#include "ms_rx.h"

#include "../lib/monocypher/monocypher.h"


/* housekeeping_interval is how often we awake to make some routine
 * like check peers for timeouts
 */
enum {
    housekeeping_start_delay = 1,
    housekeeping_interval    = 10
};



unsigned long long generate_cookie(struct ms_node* n, unsigned int ip, unsigned short port)
{
    uint8_t input[14]; /* ip+port+timeslot */
    unsigned long long cookie;
    unsigned long long timeslot = time(NULL) / cookie_lifetime;
    
    memcpy(input, &ip, sizeof(ip));
    memcpy(input + sizeof(ip), &port, sizeof(port));
    memcpy(input + sizeof(ip) + sizeof(port), &timeslot, sizeof(timeslot));

    crypto_blake2b_keyed((uint8_t*)&cookie, sizeof(cookie),
                         n->id->cookish, cookish_size,
                         input, sizeof(input));
    return cookie;
}


int verify_cookie(struct ms_node* n, unsigned int ip, unsigned short port,
                  unsigned long long cookie) 
{
    unsigned long long expected = generate_cookie(n, ip, port);
    return (expected == cookie) ? 0 : -1;
}


int send_to(int fd, unsigned int ip, unsigned short port,
             const void *buf, int len)
{
    struct sockaddr_in saddr;
    int r;

    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(ip);
    saddr.sin_port = htons(port);

    r = sendto(fd, buf, len, 0, (struct sockaddr*)&saddr, sizeof(saddr));
    if(r < 1) {
        message_perror(mlv_alert, "ALERT", "sendto");
        message(mlv_alert, "error sending %d bytes to %s",
                                    len, ipport2a(ip, port));
        return -1;
    } else if(r != len) {
        message(mlv_alert,
                "dgram len mismatch: %d to send, %d sent", len, r);
        return -1;
    } else {
        message(mlv_debug2, "sent %d bytes to %s",
                                     fd, len, ipport2a(ip, port));
    }
    return 0;
}


static void handle_unknown_dgram(struct ms_node* node,
                                 unsigned int ip, unsigned short port,
                                 unsigned char* dgram, int len)
{

}

static void handle_intro_req(struct ms_node* node,
                             unsigned int ip, unsigned short port,
                             unsigned char* dgram, int len)
{


}

static void handle_intro(struct ms_node* node,
                         unsigned int ip, unsigned short port,
                         unsigned char* dgram, int len)
{

}

static void handle_echo_req(struct ms_node* node,
                            unsigned int ip, unsigned short port,
                            unsigned char* dgram, int len)
{


}

static void handle_echo_reply(struct ms_node* node,
                              unsigned int ip, unsigned short port,
                              unsigned char* dgram, int len)
{

}

static void handle_assoc_req(struct ms_node* node,
                             unsigned int ip, unsigned short port,
                             unsigned char* dgram, int len)
{

}

static void handle_assoc_fini(struct ms_node* node,
                             unsigned int ip, unsigned short port,
                             unsigned char* dgram, int len)
{
}

static void handle_error(struct ms_node* node,
                             unsigned int ip, unsigned short port,
                             unsigned char* dgram, int len)
{
}

static void handle_plain_dgram(struct ms_node* node,
                               unsigned int ip, unsigned short port,
                               unsigned char* dgram, int len)
{
    unsigned char cmd;
    cmd = get_plain_dgram_cmd(dgram);
    message(mlv_debug, "plain dgram: cmd %02x\n", cmd);
    switch (cmd) {
        case ms_cmd_echo_req: 
            handle_echo_req(node, ip, port, dgram, len);
            break;
        case ms_cmd_echo_reply: 
            handle_echo_reply(node, ip, port, dgram, len);
            break;
        case ms_cmd_assoc_req: 
            handle_assoc_req(node, ip, port, dgram, len);
            break;
        case ms_cmd_assoc_fini: 
            handle_assoc_fini(node, ip, port, dgram, len);
            break;
        case ms_cmd_intro_req: 
            handle_intro_req(node, ip, port, dgram, len);
            break;
        case ms_cmd_intro_reply: 
            handle_intro(node, ip, port, dgram, len);
            break;
        case ms_cmd_error: 
            handle_error(node, ip, port, dgram, len);
            break;
        default:
            handle_unknown_dgram(node, ip, port, dgram, len);
            break;
    }
}

static void handle_encrypted_dgram(struct ms_node* node,
                                   unsigned int ip, unsigned short port,
                                   unsigned char* dgram, int len)
{

}


static void handle_incoming_dgram(struct ms_node* node,
                                  unsigned int ip, unsigned short port,
                                  unsigned char* dgram, int len)
{
    if(len > ms_max_dgram) {
        message(mlv_debug, 
                "dgram too long, dropping\n",
                ipport2a(ip, port));
        /* here must be some means against peers that send us such shit*/
        return;
    }

    if(dgram[0] >= ms_zb_plain_min && dgram[0] <= ms_zb_plain_max) {
        handle_plain_dgram(node, ip, port, dgram, len);
    }
    else if(dgram[0] >= ms_zb_enc_max && dgram[0] <= ms_zb_enc_max) {
        handle_encrypted_dgram(node, ip, port, dgram, len);
    }

}


static void the_fd_handler_write(struct sue_fd_handler* h) 
{

}

static void the_fd_handler_read(struct sue_fd_handler* h)
{
    unsigned char buf[2048];
    int rc;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    unsigned int ip;
    unsigned short port;
    struct ms_node *node = h->userdata;

    message(mlv_debug, "handle_recv called\n");
    rc = recvfrom(node->fd_h.fd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &addr_len);
    if(rc == -1)
        message_perror(mlv_alert, "ALERT", "handle_recv");
    ip = htonl(addr.sin_addr.s_addr);
    port = htons(addr.sin_port);

    message(mlv_debug, "received %d bytes from %s\n", rc, ipport2a(ip, port));
    
    handle_incoming_dgram(node, ip, port, buf, rc);
}


static void the_fd_handler(struct sue_fd_handler *h, int r, int w, int x)
{
    struct ms_node *node = h->userdata;

    message(mlv_debug2, "the_fd_handler called (%s)(%s)",
                        r ? "r" : "-", w ? "w" : "-");

    if(r)
        the_fd_handler_read(h);
    if(w)
        the_fd_handler_write(h);
    if(x)  /* WTF?! */
        message(mlv_debug, "the_fd_handler want exeption? WTF?\n");

    h->want_read = 1;
    h->want_write = txq_want_write(node->txq);
    h->want_except = 0;
}

static void the_timeout_hdl(struct sue_timeout_handler *hdl)
{
    struct ms_node *node = hdl->userdata;
    /* todo: */

    message(mlv_debug2, "the_timeout_hdl called");

    // peers_timer_hook(node->peers);

    node->fd_h.want_read = 1;
    node->fd_h.want_write = txq_want_write(node->txq);
    node->fd_h.want_except = 0;

    sue_timeout_set_from_now(hdl, housekeeping_interval, 0);
    sue_sel_register_timeout(node->the_selector, &node->tmo_h);
}

struct ms_node* make_node(struct sue_event_selector* s, struct ms_node_cfg* cfg)
{
    struct ms_node* node;
    node = malloc(sizeof(*node));

    node->fd_h.fd = -1;
    node->fd_h.want_read = 1;
    node->fd_h.want_write = 0;
    node->fd_h.want_except = 0;
    node->fd_h.userdata = node;
    node->fd_h.handle_fd_event = &the_fd_handler;

    node->tmo_h.userdata = node;
    node->tmo_h.handle_timeout = &the_timeout_hdl;
    node->the_selector = s;
    node->the_cfg = cfg;

    node->id = load_node_id(cfg);
    if(!node->id) {
        message(mlv_alert, "problems loading node id\n");
        free(node);
        return NULL;
    }

    node->peers = make_peer_collection(node, node->the_cfg);
    node->txq = make_transmit_queue(s);

    return node;
}


#if 0
int load_node_cfg(struct ms_node* n, const char* fname)
{
    n->the_cfg = make_node_cfg();
    return read_node_cfg_file(n->the_cfg, fname);
}
#endif

int kill_node(struct ms_node* n)
{
    
    return 0;
}


int start_node(struct ms_node *node)
{
    int sfd = make_sock(SOCK_DGRAM, 
                        node->the_cfg->listen_ip, 
                        node->the_cfg->listen_port);
    if(sfd == -1){
        message(mlv_alert, "[FATAL] Unable to create socket\n");
        return -1;
    }

    /* success */

    node->fd_h.fd = sfd;
    node->fd_h.want_read = 1;
    sue_sel_register_fd(node->the_selector, &node->fd_h);

    sue_timeout_set_from_now(&node->tmo_h, housekeeping_start_delay, 0);
    sue_sel_register_timeout(node->the_selector, &node->tmo_h);

    return 0;
}

