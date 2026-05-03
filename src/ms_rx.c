#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "ms_rx.h"
#include "ms_nodeid.h"
#include "ms_nodecfg.h"
#include "ms_comm_ctx.h"
#include "ms_txq.h"
#include "ms_peers.h"
#include "ms_keyutils.h"
#include "ms_comm_ctx.h"
#include "socks.h"

#include "addrport.h"
#include "message.h"
#include "ms_rx.h"
#include "hexdata.h"

#include "../lib/monocypher/monocypher.h"


/* housekeeping_interval is how often we awake to make some routine
 * like check peers for timeouts
 */
enum {
    housekeeping_start_delay = 1,
    housekeeping_interval    = 10,

    padded_msg_len           = 148,

    timestamp_len            = 8,

    echo_req_reserved        = 4,
    echo_reply_reserved      = 4,

    echo_reply_payload_len   = 20,
};



static unsigned long long generate_cookie(struct ms_udp_receiver* rx, unsigned int ip, unsigned short port)
{
    uint8_t input[14]; /* ip+port+timeslot */
    unsigned long long cookie;
    unsigned long long timeslot = time(NULL) / cookie_lifetime;
    
    memcpy(input, &ip, sizeof(ip));
    memcpy(input + sizeof(ip), &port, sizeof(port));
    memcpy(input + sizeof(ip) + sizeof(port), &timeslot, sizeof(timeslot));

    crypto_blake2b_keyed((uint8_t*)&cookie, sizeof(cookie),
                         rx->id->cookish, cookish_size,
                         input, sizeof(input));
    return cookie;
}


static int cookie_is_valid(struct ms_udp_receiver* rx, unsigned int ip, unsigned short port,
                         unsigned long long cookie) 
{
    unsigned long long expected = generate_cookie(rx, ip, port);
    return (expected == cookie);
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
        message_perror(mlv_alert, "send_to", "failed to send dgram");
        message(mlv_alert, "[ERROR] error sending %d bytes to %s",
                           len, ipport2a(ip, port));
        return -1;
    } else if(r != len) {
        message(mlv_alert,
                "[ERROR] dgram len mismatch: %d to send, %d sent", len, r);
        return -1;
    } else {
        message(mlv_debug2, "[DEBUG] sent %d bytes to %s",
                            fd, len, ipport2a(ip, port));
    }
    return 0;
}

static void enqueue_datagram(struct ms_udp_receiver *rx,
                             struct ms_transmit_item *msg)
{
    ms_txq_enqueue(msg);
}

static void send_plaintext_148(struct ms_udp_receiver *rx,
                               unsigned int ip, unsigned short port,
                               int cmd,
                               unsigned char *payload,
                               int payload_len)
{
    struct ms_transmit_item *msg;

    if(payload_len > 126) {
        message(mlv_alert,
                        "[ERROR] send_plaintext_148: payload too long (%d)\n",
                        payload_len);
        return;
    }

    msg = make_txitem_4ip(rx->txq, 128, 0, ip, port);

    set_plain_dgram_head(msg->buf, cmd);
    if(payload_len > 0)
        memcpy(msg->buf + 2, payload, payload_len);
    fill_noise(msg->buf + 2 + payload_len, msg->len - 2 - payload_len);
    message(mlv_debug2, "[DEBUG] sending plaintext dgram (cmd=%02x) to %s",
                        cmd, ipport2a(msg->ip, msg->port));
    enqueue_datagram(rx, msg);
}

static unsigned long long get_timestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (unsigned long)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

#if 0
static void place_timestamp(unsigned char ts[8])
{
    unsigned long long timestamp;
    timestamp = get_timestamp();
    u64_to_big_endian(ts, timestamp);
}
#endif


static void send_echo_reply(struct ms_udp_receiver* rx, unsigned int ip, unsigned short port) 
{
    unsigned char buf[echo_reply_payload_len];
    unsigned char* bufp;
    unsigned long long cookie;
    unsigned long long timestamp;


    cookie = generate_cookie(rx, ip, port);
    timestamp = get_timestamp();

    message(mlv_debug, "[DEBUG] sending echo reply to %s, "
                       "timestamp: %llu\n"
                       " cookie: %llu", 
                       timestamp,
                       cookie);

    bufp = buf;
    bufp += echo_reply_reserved;
    u64_to_big_endian(bufp, timestamp);
    bufp += timestamp_len;
    u64_to_big_endian(bufp, cookie);
    send_plaintext_148(rx, 
                       ip, port,
                       ms_cmd_echo_reply, 
                       buf, echo_reply_payload_len);
}


static void handle_unknown_dgram(struct ms_udp_receiver* rx,
                                 unsigned int ip, unsigned short port,
                                 unsigned char* dgram, int len)
{

}

static void handle_intro_req(struct ms_udp_receiver* rx,
                             unsigned int ip, unsigned short port,
                             unsigned char* data, int len)
{


}

static void handle_intro(struct ms_udp_receiver* rx,
                         unsigned int ip, unsigned short port,
                         unsigned char* dgram, int len)
{

}


static void handle_echo_req(struct ms_udp_receiver* rx,
                            unsigned int ip, unsigned short port,
                            unsigned char* data, int len)
{
    unsigned long long received_timestamp;
    received_timestamp = u64_from_big_endian(data);
    message(mlv_debug, "[DEBUG] received echo request from %s, timestamp: %llu\n", ipport2a(ip, port), received_timestamp);
    /* for now timestamp not used */
    send_echo_reply(rx, ip, port);
}

static void handle_echo_reply(struct ms_udp_receiver* rx,
                              unsigned int ip, unsigned short port,
                              unsigned char* dgram, int len)
{

}

static void handle_assoc_req(struct ms_udp_receiver* rx,
                             unsigned int ip, unsigned short port,
                             unsigned char* dgram, int len)
{

}

static void handle_assoc_fini(struct ms_udp_receiver* rx,
                             unsigned int ip, unsigned short port,
                             unsigned char* dgram, int len)
{
}

static void handle_error(struct ms_udp_receiver* rx,
                             unsigned int ip, unsigned short port,
                             unsigned char* dgram, int len)
{
}

static void handle_plain_dgram(struct ms_udp_receiver* rx,
                               unsigned int ip, unsigned short port,
                               unsigned char* dgram, int len)
{
    unsigned char cmd;
    cmd = get_plain_dgram_cmd(dgram);
    message(mlv_debug, "[DEBUG] plain dgram: cmd %02x\n", cmd);
    switch (cmd) {
        case ms_cmd_echo_req: 
            handle_echo_req(rx, ip, port, dgram+2, len-2);
            break;
        case ms_cmd_echo_reply: 
            handle_echo_reply(rx, ip, port, dgram+2, len-2);
            break;
        case ms_cmd_assoc_req: 
            handle_assoc_req(rx, ip, port, dgram+2, len-2);
            break;
        case ms_cmd_assoc_fini: 
            handle_assoc_fini(rx, ip, port, dgram+2, len-2);
            break;
        case ms_cmd_intro_req: 
            handle_intro_req(rx, ip, port, dgram+2, len-2);
            break;
        case ms_cmd_intro_reply: 
            handle_intro(rx, ip, port, dgram+2, len-2);
            break;
        case ms_cmd_error: 
            handle_error(rx, ip, port, dgram+2, len-2);
            break;
        default:
            handle_unknown_dgram(rx, ip, port, dgram, len);
            break;
    }
}

static void handle_encrypted_dgram(struct ms_udp_receiver* rx,
                                   unsigned int ip, unsigned short port,
                                   unsigned char* dgram, int len)
{

}


static void handle_incoming_dgram(struct ms_udp_receiver* rx,
                                  unsigned int ip, unsigned short port,
                                  unsigned char* dgram, int len)
{
    if(len > ms_max_dgram) {
        message(mlv_debug, 
                "[DEBUG] dgram too long, dropping\n",
                ipport2a(ip, port));
        /* here must be some means against peers that send us such shit*/
        return;
    }

    if(dgram[0] >= ms_zb_plain_min && dgram[0] <= ms_zb_plain_max) {
        handle_plain_dgram(rx, ip, port, dgram, len);
    }
    else if(dgram[0] >= ms_zb_enc_max && dgram[0] <= ms_zb_enc_max) {
        handle_encrypted_dgram(rx, ip, port, dgram, len);
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
    struct ms_udp_receiver *rx = h->userdata;

    message(mlv_debug, "[DEBUG] the_fd_handler_read called\n");
    rc = recvfrom(rx->fd_h.fd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &addr_len);
    if(rc == -1)
        message_perror(mlv_alert, "the_fd_handler_read", "recvfrom");
    ip = htonl(addr.sin_addr.s_addr);
    port = htons(addr.sin_port);

    message(mlv_debug, "[DEBUG] received %d bytes from %s\n", rc, ipport2a(ip, port));
    
    handle_incoming_dgram(rx, ip, port, buf, rc);
}


static void the_fd_handler(struct sue_fd_handler *h, int r, int w, int x)
{
    struct ms_udp_receiver *rx = h->userdata;

    message(mlv_debug, "[DEBUG] the_fd_handler called (%s)(%s)",
                       r ? "r" : "-", w ? "w" : "-");

    if(r)
        the_fd_handler_read(h);
    if(w)
        the_fd_handler_write(h);
    if(x)  /* WTF?! */
        message(mlv_debug, "[DEBUG] the_fd_handler want exeption? WTF?\n");

    h->want_read = 1;
    h->want_write = txq_want_write(rx->txq);
    h->want_except = 0;
}

static void the_timeout_hdl(struct sue_timeout_handler *hdl)
{
    struct ms_udp_receiver *rx = hdl->userdata;
    /* todo: */

    message(mlv_debug2, "[DEBUG] the_timeout_hdl called\n");

    // peers_timer_hook(rx->peers);

    rx->fd_h.want_read = 1;
    rx->fd_h.want_write = txq_want_write(rx->txq);
    rx->fd_h.want_except = 0;

    sue_timeout_set_from_now(hdl, housekeeping_interval, 0);
    sue_sel_register_timeout(rx->the_selector, &rx->tmo_h);
}

struct ms_udp_receiver* make_udp_receiver(struct sue_event_selector* s, struct ms_node_cfg* cfg)
{
    struct ms_udp_receiver* rx;
    rx = malloc(sizeof(*rx));

    rx->fd_h.fd = -1;
    rx->fd_h.want_read = 1;
    rx->fd_h.want_write = 0;
    rx->fd_h.want_except = 0;
    rx->fd_h.userdata = rx;
    rx->fd_h.handle_fd_event = &the_fd_handler;

    rx->tmo_h.userdata = rx;
    rx->tmo_h.handle_timeout = &the_timeout_hdl;
    rx->the_selector = s;
    rx->the_cfg = cfg;

    rx->id = load_node_id(cfg);
    if(!rx->id) {
        message(mlv_alert, "[DEBUG] problems loading node id\n");
        free(rx);
        return NULL;
    }

    rx->peers = make_peer_collection(rx, rx->the_cfg);
    rx->txq = make_transmit_queue(s);

    return rx;
}


#if 0
int load_node_cfg(struct ms_udp_receiver* rx, const char* fname)
{
    rx->the_cfg = make_node_cfg();
    return read_node_cfg_file(rx->the_cfg, fname);
}
#endif

int start_udp_receiver(struct ms_udp_receiver *rx)
{
    int sfd = make_sock(SOCK_DGRAM, 
                        rx->the_cfg->listen_ip, 
                        rx->the_cfg->listen_port);
    if(sfd == -1){
        message(mlv_alert, "[FATAL] Unable to create socket\n");
        return 0;
    }

    /* success */

    rx->fd_h.fd = sfd;
    rx->fd_h.want_read = 1;
    sue_sel_register_fd(rx->the_selector, &rx->fd_h);

    sue_timeout_set_from_now(&rx->tmo_h, housekeeping_start_delay, 0);
    sue_sel_register_timeout(rx->the_selector, &rx->tmo_h);

    return 1;
}

