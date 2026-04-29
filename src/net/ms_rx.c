#include "ms_rx.h"
#include "addrport.h"
#include "../message.h"
#include "ms_comm_ctx.h"
#include "../../lib/monocypher/monocypher.h"

#include <arpa/inet.h>
#include <string.h>
#include <time.h>

unsigned long long generate_cookie(struct ms_node* n, unsigned int ip, unsigned short port)
{
    uint8_t input[14]; /* ip+port+timeslot */
    unsigned long long cookie;
    unsigned long long timeslot = time(NULL) / cookie_lifetime;
    
    memcpy(input, &ip, sizeof(ip));
    memcpy(input + sizeof(ip), &port, sizeof(port));
    memcpy(input + sizeof(ip) + sizeof(port), &timeslot, sizeof(timeslot));

    crypto_blake2b_keyed((uint8_t*)&cookie, sizeof(cookie),
                         n->cookish, cookish_size,
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

static void handle_assoc(struct ms_node* node,
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
        case ms_cmd_echo_request: 
            handle_echo_req(node, ip, port, dgram, len);
            break;
        case ms_cmd_echo_reply: 
            handle_echo_reply(node, ip, port, dgram, len);
            break;
        case ms_cmd_assoc: 
            handle_assoc(node, ip, port, dgram, len);
            break;
        case ms_cmd_intro_req: 
            handle_intro_req(node, ip, port, dgram, len);
            break;
        case ms_cmd_intro_reply: 
            handle_intro(node, ip, port, dgram, len);
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
 
int do_recv(struct ms_node* node) {
    unsigned char buf[2048];
    int rc;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    unsigned int ip;
    unsigned short port;

    if(node->state != msns_active) return -1;


    message(mlv_debug, "handle_recv called\n");
    rc = recvfrom(node->fd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &addr_len);
    if(rc == -1) {
        message_perror(mlv_alert, "ALERT", "handle_recv");
        return rc;
    }
    ip = htonl(addr.sin_addr.s_addr);
    port = htons(addr.sin_port);

    message(mlv_debug, "received %d bytes from %s\n", rc, ipport2a(ip, port));
    
    handle_incoming_dgram(node, ip, port, buf, rc);
    return 0;
}

