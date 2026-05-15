#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "ms_rx.h"
#include "ms_nodeid.h"
#include "ms_nodecfg.h"
#include "ms_comm_ctx.h"
#include "ms_txq.h"
#include "ms_peers.h"
#include "keyutils.h"
#include "ms_comm_ctx.h"
#include "ms_kndb.h"
#include "socks.h"

#include "addrport.h"
#include "log.h"
#include "ms_rx.h"
#include "hexdata.h"

#include "../lib/monocypher/monocypher.h"

enum {
    housekeeping_start_delay = 1,
    housekeeping_interval    = 1,

    padded_msg_size          = 148,

    timestamp_size           = 8,


    echo_req_reserved        = 4,
    echo_reply_reserved      = 4,
    echo_req_payload_size    = 12,
    echo_reply_payload_size  = 20,

    assoc_req_payload_size   = 66,
    assoc_fini_payload_size  = 26,
};



void obfuscate(unsigned char *buf, int size)
{
    int i;
    *buf ^= 0x5a;
    for(i = 1; i < size; i++) {
        unsigned char pb, mask, cb, sft;

        pb = buf[i-1];
        mask = (pb & 0xf8) | ((pb >> 5) & 0x07);
        mask ^= (i & 0x0f) | ((i<<4) & 0xf0);
        sft = (pb & 0x07) ^ (i & 0x07);
        cb = buf[i] ^ mask;
        if(sft)
            cb = (cb >> sft) | ((cb << (8-sft)) & (0xff << (8-sft)));
        buf[i] = cb;
    }
}

void deobfuscate(unsigned char *buf, int size)
{
    int i;
    for(i = size-1; i >= 1; i--) {
        unsigned char pb, mask, cb, sft;

        pb = buf[i-1];
        mask = (pb & 0xf8) | ((pb >> 5) & 0x07);
        mask ^= (i & 0x0f) | ((i<<4) & 0xf0);
        sft = 8 - ((pb & 0x07) ^ (i & 0x07));
        cb = buf[i];
        if(sft)
            cb = (cb >> sft) | ((cb << (8-sft)) & (0xff << (8-sft)));
        buf[i] = cb ^ mask;
    }
    *buf ^= 0x5a;
}

void ms_rx_peer_gone(struct ms_udp_receiver *rx,
                     struct ms_peer *peer)
{
    txq_peer_gone(rx->txq, peer);
}

static unsigned long long generate_token(struct ms_udp_receiver* rx, unsigned int ip, unsigned short port)
{
    uint8_t input[14]; /* ip+port+timeslot */
    unsigned long long token;
    unsigned long long timeslot = timemark_sec(rx->peers) / token_lifetime;
    
    memcpy(input, &ip, sizeof(ip));
    memcpy(input + sizeof(ip), &port, sizeof(port));
    memcpy(input + sizeof(ip) + sizeof(port), &timeslot, sizeof(timeslot));

    crypto_blake2b_keyed((uint8_t*)&token, sizeof(token),
                         rx->comctx.token_key, token_key_size,
                         input, sizeof(input));
    return token;
}


static int token_is_valid(struct ms_udp_receiver* rx, unsigned int ip, unsigned short port,
                         unsigned long long token) 
{
    unsigned long long expected = generate_token(rx, ip, port);
    return (expected == token);
}


int send_to(int fd, 
            unsigned int ip, unsigned short port,
            const void *buf, int len)
{
    struct sockaddr_in saddr;
    int r;

    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(ip);
    saddr.sin_port = htons(port);

    r = sendto(fd, buf, len, 0, (struct sockaddr*)&saddr, sizeof(saddr));
    if(r < 1) {
        log_perror(llv_alert, "send_to", "failed to send dgram");
        log_msg(llv_alert, "error sending %d bytes to %s",
                           len, ipport2a(ip, port));
        return -1;
    } else if(r != len) {
        log_msg(llv_alert,
                "dgram len mismatch: %d to send, %d sent",
                len, r);
        return -1;
    } else {
        log_msg(llv_debug2, "sent %d bytes to %s",
                len, ipport2a(ip, port));
    }
    return 0;
}

static void enqueue_datagram(struct ms_udp_receiver *rx,
                             struct ms_transmit_item *msg)
{
    txq_enqueue(msg);
}

static void send_plaintext_148(struct ms_udp_receiver* rx,
                               struct ms_peer* peer,
                               unsigned int ip, unsigned short port,
                               int cmd,
                               const unsigned char* payload,
                               int payload_len)
{
    struct ms_transmit_item *msg;

    if(payload_len > 146) {
        log_msg(llv_alert,
                "send_plaintext_148: payload too long (%d)",
                payload_len);
        return;
    }

    if(peer)
        msg = make_txitem_4peer(rx->txq, 148, 0, peer);
    else
        msg = make_txitem_4ip(rx->txq, 148, 0, ip, port);


    set_plain_dgram_head(msg->buf, cmd);
    if(payload_len > 0)
        memcpy(msg->buf + 2, payload, payload_len);
    fill_noise(msg->buf + 2 + payload_len, msg->len - 2 - payload_len);
    log_msg(llv_debug2, 
            "sending plaintext dgram (cmd=%02x) to %s",
            cmd, ipport2a(msg->ip, msg->port));
    enqueue_datagram(rx, msg);
}

static void send_echo_reply(struct ms_udp_receiver* rx, unsigned int ip, unsigned short port) 
{
    unsigned char buf[echo_reply_payload_size];
    unsigned char* bufp;
    unsigned long long token;
    unsigned long long timemark;

    token = generate_token(rx, ip, port);
    timemark = timemark_sec(rx->peers);

    log_msg(llv_debug,
            "sending echo_reply to %s",
            ipport2a(ip, port));

    bufp = buf;
    bufp += echo_reply_reserved;
    u64_to_big_endian(bufp, timemark);
    bufp += timestamp_size;
    u64_to_big_endian(bufp, token);
    send_plaintext_148(rx, 
                       NULL,
                       ip, port,
                       ms_cmd_echo_reply, 
                       buf, echo_reply_payload_size);
}

static void send_echo_request(struct ms_udp_receiver* rx, struct ms_peer* peer) 
{
    unsigned char buf[echo_req_payload_size];
    unsigned char* bufp;
    unsigned long long timemark;
    unsigned int ip;
    unsigned short port;

    peer_get_addr(peer, &ip, &port);

    timemark = timemark_sec(rx->peers);

    log_msg(llv_debug, "sending echo_req to %s",
            ipport2a(ip, port));

    bufp = buf;
    memset(bufp, 0, echo_req_reserved);
    bufp += echo_req_reserved;
    u64_to_big_endian(bufp, timemark);
    send_plaintext_148(rx, 
                       peer,
                       ip, port,
                       ms_cmd_echo_request, 
                       buf, echo_req_payload_size);
}

static void send_assoc_request(struct ms_udp_receiver* rx, struct ms_peer* peer)
{
    unsigned char buf[assoc_req_payload_size + sign_size];
    unsigned char *token, *id, *kex, *cookie, *timemark; 
    unsigned char* sign = buf + assoc_req_payload_size;
    unsigned int ip;
    unsigned short port;
    /*
        assoc_request
        2..9        8       received token
        10..19      10      our_id
        20..51      32      kex
        52..59      8       cookie
        60..67      8       timemark
        68..131     64      our sign for ALL above
        132..147            random padding
    */

    memset(buf, 0, sizeof(buf));
    peer_get_addr(peer, &ip, &port);

    token = buf;
    id = token + token_size;
    kex = id + node_id_size;
    cookie = kex + kex_public_size;
    timemark = cookie + 8;

    u64_to_big_endian(token, peer_token(peer));
    memcpy(id, peer_id(peer), node_id_size);
    memcpy(kex, rx->comctx.kex_public, kex_public_size);
    memcpy(cookie, peer_cookie(peer), 8);
    u64_to_big_endian(timemark, timemark_sec(rx->peers));
    obfuscate(timemark, 8);

    crypto_eddsa_sign(sign,
                      rx->comctx.identity->master_secret_key,
                      buf,
                      assoc_req_payload_size);

    log_msg(llv_debug, "sending assoc_req to %s",
            ipport2a(ip, port));
#ifdef _RX_DEBUG
    log_msg_bald(llv_debug, "token:    %s",
            hexdata2a(token, token_size));
    log_msg_bald(llv_debug, "id:       %s",
            hexdata2a(id, node_id_size));
    log_msg_bald(llv_debug, "kex:      %s",
            hexdata2a(kex, kex_public_size));
    log_msg_bald(llv_debug, "cookie:   %s",
            hexdata2a(cookie, 8));
    log_msg_bald(llv_debug, "timemark: %s",
            hexdata2a(timemark, 8));
#endif

    send_plaintext_148(rx, 
                       peer,
                       ip, port,
                       ms_cmd_assoc_request, 
                       buf, assoc_req_payload_size);
}

static void send_semiencrypted_with_key(struct ms_udp_receiver* rx,
                                        struct ms_peer* peer,
                                        const unsigned char* encrypt_key,
                                        int cmd,
                                        unsigned char* payload,
                                        int payload_len)
{
    struct ms_transmit_item *msg;
    unsigned char nonce[cipher_nonce_total];
    unsigned char *bufp, *mac, *ct;
    int ctpos, ctsize, totalsize;

    totalsize = 58 + payload_len; /*cmd + kex + nonce + mac*/

    msg = make_txitem_4peer(rx->txq,
                            totalsize > 148 ? totalsize : 148, 0, peer);

    bufp = msg->buf;
    set_plain_dgram_head(bufp, cmd);
    bufp += 2;
    memcpy(bufp, rx->comctx.kex_public, kex_public_size);
    bufp += kex_public_size;

    memset(nonce, 0, cipher_nonce_offset);
    comctx_fill_nonce(&rx->comctx, nonce + cipher_nonce_offset);
    memcpy(bufp, nonce + cipher_nonce_offset, cipher_nonce_used);
    obfuscate(bufp, cipher_nonce_used);
    bufp += cipher_nonce_used;

    mac = bufp;
    bufp += cipher_mac_size;
    ct = bufp;

    ctpos = ct - msg->buf;
    ctsize = msg->len - ctpos;

    memcpy(ct, payload, payload_len);
    bufp += payload_len;

    if(msg->len - ctpos - payload_len > 0) 
        fill_noise(bufp, msg->len - ctpos - payload_len);

    crypto_aead_lock(ct, mac, encrypt_key, nonce, NULL, 0, ct, ctsize);


    log_msg(llv_debug2, "sending semiencrypted dgram (cmd=%02x, size=%d) to %s",
            cmd, msg->len, ipport2a(msg->ip, msg->port));
#ifdef _RX_DEBUG
    
    log_msg(llv_debug2, "kex:   %s",
            hexdata2a(msg->buf + 2, public_key_size));
    log_msg(llv_debug2, "nonce: %s",
            hexdata2a(nonce + cipher_nonce_offset, cipher_nonce_used));
    log_msg(llv_debug2, "mac:   %s",
            hexdata2a(mac, cipher_mac_size));
    log_msg(llv_debug2, "ct:    %s",
            hexdata2a(ct, ctsize));
#endif

    enqueue_datagram(rx, msg);
}

static void send_assoc_fini(struct ms_udp_receiver* rx, struct ms_peer* peer)
{
    unsigned char payload[assoc_fini_payload_size + sign_size];
    unsigned int ip;
    unsigned short port;
    unsigned char *id, *cookie, *timemark, *sign;

    peer_get_addr(peer, &ip, &port);

    id = payload;
    cookie = id + node_id_size;
    timemark = cookie + 8;
    sign = timemark + 8;

    memcpy(id, rx->comctx.identity->node_id, node_id_size);
    memcpy(cookie, peer_cookie(peer), 8);
    u64_to_big_endian(timemark, timemark_sec(rx->peers));
    obfuscate(timemark, 8);
    crypto_eddsa_sign(sign, 
                      rx->comctx.identity->master_secret_key,
                      payload,
                      assoc_fini_payload_size);

    log_msg(llv_debug, "sending assoc_fini to %s",
            ipport2a(ip, port));
#ifdef _RX_DEBUG
    log_msg_bald(llv_debug, "id:       %s",
                 hexdata2a(id, node_id_size));
    log_msg_bald(llv_debug, "cookie:   %s",
                 hexdata2a(cookie, assoc_fini_payload_size));
    log_msg_bald(llv_debug, "timemark: %s",
                 hexdata2a(timemark, 8));
    log_msg_bald(llv_debug, "sign:     %s",
                 hexdata2a(sign, sign_size));
#endif
    send_semiencrypted_with_key(rx, peer, 
                                peer_encrypt_key(peer), 
                                ms_cmd_assoc_fini, 
                                payload, sizeof(payload));
}

static void send_encrypted(struct ms_udp_receiver *rx,
                           struct ms_peer *peer,
                           const unsigned char *payload, int payload_len)
{
    struct ms_transmit_item *msg;
    unsigned char *mac, *ct;
    int dgram_size, padding, totalsize;

    dgram_size = cipher_nonce_used + cipher_mac_size + payload_len;
    if(dgram_size < ms_min_dgram) {
        padding = ms_min_dgram - dgram_size;
        dgram_size = ms_min_dgram;
    } else {
        padding = 0;
    }
    totalsize = cipher_nonce_total + cipher_mac_size + payload_len + padding;

    msg = make_txitem_4peer(rx->txq,
                            totalsize, cipher_nonce_offset, peer);

    memset(msg->buf, 0, cipher_nonce_offset);
    comctx_fill_nonce(&rx->comctx , msg->buf + cipher_nonce_offset);

    mac = msg->buf + cipher_nonce_total;
    ct = mac + cipher_mac_size;
    memcpy(ct, payload, payload_len);

    if(padding > 0) 
        fill_noise(ct + payload_len, padding);

    crypto_aead_lock(ct, mac, peer_encrypt_key(peer), msg->buf, NULL, 0,
                     ct, payload_len + padding);

    obfuscate(msg->buf + cipher_nonce_offset, cipher_nonce_used);

    if(msg->buf[msg->offset] > ms_zb_enc_max) {
        msg->offset--;
        msg->buf[msg->offset] =
            rand_from_range(ms_zb_enc_min, ms_zb_enc_max);
    }

    log_msg(llv_debug2,
            "sending encrypted dgram (cmd=%02x, size=%d/%d) to %s",
            payload[0], msg->len - msg->offset, payload_len,
            ipport2a(msg->ip, msg->port));
#ifdef _RX_DEBUG
    log_msg_bald(llv_debug2, 
                 "nonce: %s",
                 hexdata2a(msg->buf + cipher_nonce_offset, cipher_nonce_used));
    log_msg_bald(llv_debug2, 
                 "mac:   %s",
                 hexdata2a(msg->buf + cipher_nonce_total, cipher_mac_size));
#endif

    enqueue_datagram(rx, msg);
}

static void send_enc_keepalive(struct ms_udp_receiver* rx, struct ms_peer* peer)
{
    unsigned int ip;
    unsigned short port;
    peer_get_addr(peer, &ip, &port);
    log_msg(llv_debug,
            "sending keepalive to %s",
            ipport2a(ip, port));
    unsigned char payload = ms_cmd_keep_alive;
    send_encrypted(rx, peer, &payload, 1);
}

static void send_enc_imalive(struct ms_udp_receiver* rx, struct ms_peer* peer)
{
    unsigned char payload = ms_cmd_im_alive;
    send_encrypted(rx, peer, &payload, 1);
}


static void handle_unknown_dgram(struct ms_udp_receiver* rx,
                                 unsigned int ip, unsigned short port,
                                 const unsigned char* body, int len)
{
    log_msg(llv_debug, 
            "got unknown shit from %s, dropping",
            ipport2a(ip, port));
}

static void handle_intro_req(struct ms_udp_receiver* rx,
                             unsigned int ip, unsigned short port,
                             const unsigned char* body, int len)
{


}

static void handle_intro(struct ms_udp_receiver* rx,
                         unsigned int ip, unsigned short port,
                         const unsigned char* body, int len)
{

}


static void handle_echo_req(struct ms_udp_receiver* rx,
                            unsigned int ip, unsigned short port,
                            const unsigned char* data, int len)
{
    log_msg(llv_debug, 
            "echo request from %s, will reply",
            ipport2a(ip, port));
    send_echo_reply(rx, ip, port);
}

static void handle_echo_reply(struct ms_udp_receiver* rx,
                              unsigned int ip, unsigned short port,
                              const unsigned char* body, int len)
{
    struct ms_peer* peer;
    unsigned long long received_token;
    int assoc_status;

    peer = get_peer_record(rx->peers, ip, port, 0);
    if(!peer) {
        log_msg(llv_debug, 
                "stray echo reply from %s, dropping",
                ipport2a(ip, port));
        return;
    }
    /* TODO:
        we will send error in return
        but this will be added later (i hope)
    */
    
    assoc_status = peer_assoc_status(peer);
    switch (assoc_status) {
        case as_echo_request_sent: 
            break;
        case as_established:
            log_msg(llv_debug,
                    "ignoring unrequested echo_reply from %s (assoc. established)",
                    peer_description(peer));
            return;
        case as_gave_up: 
        case as_none: 
        case as_not_desired: 
        case as_assoc_request_sent: 
        case as_assoc_fini_sent: 
            log_msg(llv_debug,
                    "didn't expect echo_reply from %s, dropping",
                    ipport2a(ip, port));
            return;
        default:
            log_msg(llv_debug,
                    "unknown value (%d) for assoc status of %s",
                    assoc_status, peer_description(peer));
            return;
    }

    log_msg(llv_debug, 
            "echo_reply from %s, will associate",
            ipport2a(ip, port));

    received_token = u64_from_big_endian(body + echo_reply_reserved + timestamp_size);
    peer_set_token(peer, received_token);
    
    peer_set_last_tm(peer, timemark_sec(rx->peers));
    peer_generate_new_cookie(peer);
    peer_set_assoc_status(peer, as_assoc_request_sent);
    send_assoc_request(rx, peer);
}


static void handle_assoc_req(struct ms_udp_receiver* rx,
                             unsigned int ip, unsigned short port,
                             unsigned char* body, int len)
{
    int kndbres, r;
    int assoc_status;
    struct ms_peer* peer;
    unsigned long long received_token;
    unsigned long long remote_tm;
    unsigned long long local_now;
    unsigned char *token, *remote_id, *kex, *cookie, *timemark;
    unsigned char *sign = body + assoc_req_payload_size;
    unsigned char remote_pubkey[public_key_size];


    if(len != 146) {
        log_msg(llv_debug, 
                "ignoring assoc_req from %s (msg len mismatch)",
                ipport2a(ip, port));
        return;
    }
    /*
        assoc_request
        2..9        8       received token
        10..19      10      id
        20..51      32      kex
        52..59      8       cookie
        60..67      8       timemark
        68..131     64      sign for ALL above
        132..147            random padding
    */
    token = body;
    remote_id = token + token_size;
    kex = remote_id + node_id_size;
    cookie = kex + kex_public_size;
    timemark = cookie + 8;
    deobfuscate(timemark, 8);

    log_msg(llv_debug, "received assoc_req from %s",
            ipport2a(ip, port));

#ifdef _RX_DEBUG
    log_msg_bald(llv_debug, "token:    %s",
                 hexdata2a(token, token_size));
    log_msg_bald(llv_debug, "id:       %s",
                 hexdata2a(remote_id, node_id_size));
    log_msg_bald(llv_debug, "kex:      %s",
                 hexdata2a(kex, kex_public_size));
    log_msg_bald(llv_debug, "cookie:   %s",
                 hexdata2a(cookie, 8));
    log_msg_bald(llv_debug, "timemark: %s",
                 hexdata2a(timemark, 8));
    log_msg_bald(llv_debug, "sign:     %s",
                 hexdata2a(sign, sign_size));
#endif

    received_token = u64_from_big_endian(token);
    if(!token_is_valid(rx, ip, port, received_token))
    {
        log_msg(llv_debug, 
                "ignoring assoc_req from %s (invalid token)",
                ipport2a(ip, port));
        /* TODO: we DEFINITELY need to send error and probably set cooldown for this peer */
        return;
    }


    peer = get_peer_record(rx->peers, ip, port, 1);
    assoc_status = peer_assoc_status(peer);
    switch (assoc_status) {
        case as_none: /* that's good*/
        case as_echo_request_sent:
        case as_assoc_fini_sent: /* maybe they lost out fini, so let's try resending */
        break;
        case as_gave_up: /* that's also good*/
            peer_set_assoc_status(peer, as_none);
            break;  
        case as_established: /* who not... if sign will be valid we can try re-assoc*/
            if(!node_id_is_same(peer, remote_id)) {
                log_msg(llv_debug,
                        "ignoring re-assoc_req from %s (node id differs)",
                        ipport2a(ip, port));
                /* TODO: we probably need to send error*/
                return;
            }
            break;
        case as_not_desired:
            log_msg(llv_debug, 
                    "didn't expect assoc_req from %s, dropping",
                    ipport2a(ip, port));
            /* TODO: we DEFINITELY need to send error*/
            return;
        default:
            log_msg(llv_debug,
                    "unknown value (%d) for assoc status of %s",
                    assoc_status, peer_description(peer));
            return;
    }


    remote_tm = u64_from_big_endian(timemark);
    local_now = timemark_sec(rx->peers);
    if (remote_tm < local_now - timemark_gap || remote_tm > local_now + timemark_gap) {
        log_msg(llv_debug,
                "ignoring assoc_req from %s (invalid timestamp)",
                ipport2a(ip, port));
        return;
    }

    if (remote_tm <= peer_get_last_tm(peer) || 
        memcmp(cookie, peer_cookie(peer), 8) == 0) {
        log_msg(llv_debug,
                "ignoring assoc_req from %s (timestamp replay)",
                ipport2a(ip, port));
        return;
    }

    /* now we can try to identify it*/

    kndbres = kndb_get_node(rx->kndb, remote_id, remote_pubkey);
    if (kndbres != kndb_res_success) {
        /* TODO: will switch with detailed log_msgs later (i hope)*/
        log_msg(llv_debug,
                "ignoring assoc_req from %s (kndb refused)",
                ipport2a(ip, port));
        /* TODO: we DEFINITELY need to send error*/
        return;
    }

    r = crypto_eddsa_check(sign, remote_pubkey, 
                           body, assoc_req_payload_size);
    if(!r){
        log_msg(llv_debug, 
                "ignoring assoc_req from %s (invalid sign)",
                ipport2a(ip, port));
        /* TODO: we DEFINITELY need to send error*/
        return;
    }
    
    /* if sing is valid we don't care if it's 1st assoc or re-assoc*/

    remote_tm = u64_from_big_endian(timemark);
    if (remote_tm <= peer_get_last_tm(peer)) {
        log_msg(llv_debug, 
                "ignoring assoc_req from %s (invalid timestamp)",
                ipport2a(ip, port));
        /* TODO: we DEFINITELY need to send error and cooldown and BAN them and launch NUKE there*/
        return;
    }

    r = peer_set_identity(peer, remote_id, remote_pubkey);
    if(!r) {
        log_msg(llv_debug, 
                "giving up association with %s",
                ipport2a(ip, port));
        peer_set_assoc_status(peer, as_gave_up);
        return;
    }

    if(assoc_status == as_established) {
        log_msg(llv_normal, 
                "re-associate request from %s, will reply",
                ipport2a(ip, port));
        txq_peer_gone(rx->txq, peer);
    }
    else {
        log_msg(llv_normal, 
                "associate request from %s, will reply",
                ipport2a(ip, port));
    }


    /* all goochi setting things now*/
    
    peer_set_kex_public(rx->peers, peer, kex, 1);
    peer_set_cookie(peer, cookie);
    peer_set_last_tm(peer, remote_tm);
    update_peer_last_rx(peer);
    peer_set_assoc_status(peer, as_assoc_fini_sent);
    send_assoc_fini(rx, peer);
}

static void handle_assoc_fini(struct ms_udp_receiver* rx,
                                     unsigned int ip, unsigned short port,
                                     unsigned char* payload,
                                     int payload_len)
{
    int r, kndbres, key_still_same, assoc_status;
    struct ms_peer *peer;
    unsigned char full_nonce[cipher_nonce_total];
    unsigned char decrypt_key[cipher_key_size];
    unsigned char remote_pubkey[public_key_size];
    const unsigned char *nonce, *mac;
    unsigned char *ct, *remote_id, *timemark, *cookie, *sign;

    peer = get_peer_record(rx->peers, ip, port, 1);

    nonce = payload + kex_public_size;

    if(payload_len != 146) {
        log_msg(llv_debug, 
                "ignoring assoc_fini from %s (msg len mismatch)",
                ipport2a(ip, port));
        return;
    }

    peer = get_peer_record(rx->peers, ip, port, 0);
    if (!peer) {
        log_msg(llv_debug, 
                "ignoring stray assoc_fini from %s",
                ipport2a(ip, port));
        return;
    }

    assoc_status = peer_assoc_status(peer);
    switch (assoc_status) {
        case as_assoc_request_sent:
            break;  /* that's what we need */
        case as_none:
        case as_gave_up:
        case as_not_desired:
        case as_echo_request_sent:
            log_msg(llv_debug, 
                    "didn't expect assoc_fini from %s, dropping",
                    ipport2a(ip, port));
            return;
        default:
            log_msg(llv_debug,
                    "unknown value (%d) for assoc status of %s",
                    assoc_status, peer_description(peer));
            return;
    }

    key_still_same = peer_kex_public_is_same(peer, payload);

    derive_cipher_keys(rx->comctx.kex_secret, rx->comctx.kex_public,
                       payload, NULL, decrypt_key);

    mac = nonce + cipher_nonce_used;
    ct = payload + kex_public_size + cipher_nonce_used + cipher_mac_size;
    memset(full_nonce, 0, cipher_nonce_offset);
    memcpy(full_nonce + cipher_nonce_offset, nonce, cipher_nonce_used);
    deobfuscate(full_nonce + cipher_nonce_offset, cipher_nonce_used);
    r = crypto_aead_unlock(ct, mac, decrypt_key, full_nonce, NULL, 0,
                           ct, payload_len - (ct - payload));
    if(r != 0) {
        log_msg(llv_debug, 
                "ignoring assoc_fini from %s (failed to decryp)", 
                ipport2a(ip, port));
        return;
    }
    crypto_wipe(decrypt_key, sizeof(decrypt_key));


    if(key_still_same) {
        r = peer_check_update_nonce(peer, full_nonce + cipher_nonce_offset,
                                    "handle_associate_request");
        if(!r)
            return;
    }

    remote_id = ct;
    cookie = remote_id + node_id_size;
    timemark = cookie + 8;
    sign = cookie + 8;
    deobfuscate(timemark, 8);

    log_msg(llv_debug, "received assoc_fini from %s",
            ipport2a(ip, port));

#ifdef _RX_DEBUG
    log_msg_bald(llv_debug, "id:       %s",
        hexdata2a(remote_id, node_id_size));
    log_msg_bald(llv_debug, "cookie:   %s",
            hexdata2a(cookie, 8));
    log_msg_bald(llv_debug, "timemark: %s",
            hexdata2a(timemark, 8));
    log_msg_bald(llv_debug, "sign:     %s",
            hexdata2a(sign, sign_size));
#endif

    kndbres = kndb_get_node(rx->kndb, remote_id, remote_pubkey);
    if (kndbres != kndb_res_success) {
        log_msg(llv_debug, 
                "ignoring assoc_fini from %s (kndb refused)",
                ipport2a(ip, port));
        return;
    }
    r = crypto_eddsa_check(sign, 
                           remote_pubkey, 
                           ct, assoc_fini_payload_size);
    if (!r) {
        log_msg(llv_debug, 
                "ignoring assoc_fini from %s (invalid sign)", 
                ipport2a(ip, port));
        return;
    }

    r = peer_check_cookie(peer, cookie);
    if (!r) {
        log_msg(llv_debug, 
                "ignoring assoc_fini from %s (invalid cookie)",
                ipport2a(ip, port));
        return;
    }

    r = peer_set_identity(peer, remote_id, remote_pubkey);
    if (!r) {
        log_msg(llv_debug, "giving up association with %s",
                ipport2a(ip, port));
        peer_set_assoc_status(peer, as_gave_up);
        return;
    }

    r = peer_set_kex_public(rx->peers, peer, payload, 0);
    if(!r) {
        /* should never happen */
        log_msg(llv_alert,
                "refused to set kex from assoc_fini for %s? WTF?",
                peer_description(peer));
        return;
    }

    log_msg(llv_normal, 
            "association with %s established",
            peer_description(peer));

    update_peer_last_rx(peer);
    peer_generate_new_cookie(peer);
    peer_set_last_tm(peer, u64_from_big_endian(timemark));
    peer_set_assoc_status(peer, as_established);
    send_enc_keepalive(rx, peer);
}


static void handle_error(struct ms_udp_receiver* rx,
                         unsigned int ip, unsigned short port,
                         const unsigned char* dgram, int len)
{
}

static void handle_plain_dgram(struct ms_udp_receiver* rx,
                               unsigned int ip, unsigned short port,
                               unsigned char* dgram, int len)
{
    unsigned char cmd;
    cmd = get_plain_dgram_cmd(dgram);
    log_msg(llv_debug, "plain dgram: cmd %02x", cmd);
    switch (cmd) {
        case ms_cmd_echo_request: 
            handle_echo_req(rx, ip, port, dgram+2, len-2);
            break;
        case ms_cmd_echo_reply: 
            handle_echo_reply(rx, ip, port, dgram+2, len-2);
            break;
        case ms_cmd_assoc_request: 
            handle_assoc_req(rx, ip, port, dgram+2, len-2);
            break;
        case ms_cmd_assoc_fini: 
            handle_assoc_fini(rx, ip, port, dgram+2, len-2);
            break;
        case ms_cmd_intro_request: 
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


static void handle_enc_keepalive(struct ms_udp_receiver* rx,
                                struct ms_peer* peer)
{
    log_msg(llv_debug, "received keepalive from %s",
                                peer_description(peer));
    send_enc_imalive(rx, peer);
}

static void handle_enc_imalive(struct ms_udp_receiver* rx,
                               struct ms_peer* peer)
{
    log_msg(llv_debug, "received im alive from %s",
                                peer_description(peer));
}

static void handle_enc_data(struct ms_udp_receiver* rx,
                            struct ms_peer* peer,
                            const unsigned char* data, int len)
{
    log_msg(llv_debug, "received data from %s",
                                peer_description(peer));
}


static void handle_encrypted_dgram(struct ms_udp_receiver* rx,
                                   unsigned int ip, unsigned short port,
                                   unsigned char* dgram, int len)
{
    int r;
    int assoc_status;
    struct ms_peer *peer;
    unsigned char nonce[cipher_nonce_total];
    unsigned char *mac = dgram + cipher_nonce_used;
    unsigned char *ct = mac + cipher_mac_size;
    int ctlen = len - cipher_mac_size - cipher_nonce_used;

    peer = get_peer_record(rx->peers, ip, port, 0);
    if(!peer) {
        log_msg(llv_info,
            "stray encrypted dgram from %s, dropping",
            ipport2a(ip, port));
        /* TODO: countermeasures?*/
        return;
    }

    memset(nonce, 0, cipher_nonce_offset);
    memcpy(nonce + cipher_nonce_offset, dgram, cipher_nonce_used);
    deobfuscate(nonce + cipher_nonce_offset, cipher_nonce_used);

    r = crypto_aead_unlock(ct, mac, peer_decrypt_key(peer), nonce, NULL, 0,
                           ct, ctlen);
    if(r != 0) {
        log_msg(llv_info, "failed to decrypt dgram from %s",
                peer_description(peer));
        log_msg(llv_debug, "decrypt_key: %s",
            hexdata2a(peer_decrypt_key(peer), cipher_key_size));
        /*
            TODO: sende change keys request it's good idea, but for now ignoring it
        */
        return;
    }

    log_msg(llv_debug, "encrypted dgram: cmd %02x", ct[0]);

    r = peer_check_update_nonce(peer, nonce + cipher_nonce_offset,
                                "handle_encrypted_dgram");
    if(!r)
        return;

    update_peer_last_rx(peer);
    assoc_status = peer_assoc_status(peer);
    switch (assoc_status) {
    case as_established: 
        break;
    case as_assoc_fini_sent:
        peer_set_assoc_status(peer, as_established);
        log_msg(llv_normal,
                "association with %s established",
                peer_description(peer));
        break;
    
    }
    if(peer_assoc_status(peer) != as_established) {
    }

    switch(ct[0]) {
    case ms_cmd_keep_alive:
        handle_enc_keepalive(rx, peer);
        break;
    case ms_cmd_im_alive:
        handle_enc_imalive(rx, peer);
        break;
    case ms_cmd_data:
        handle_enc_data(rx, peer, ct+1, ctlen-1);
        break;
    default:
            /*error*/
        break;
    }
}


static void handle_incoming_dgram(struct ms_udp_receiver* rx,
                                  unsigned int ip, unsigned short port,
                                  unsigned char* dgram, int len)
{
    if(len > ms_max_dgram) {
        log_msg(llv_debug, 
                "dgram too long, dropping",
                ipport2a(ip, port));
        /* here must be some means against peers that send us such shit*/
        return;
    }

    if(dgram[0] >= ms_zb_plain_min && dgram[0] <= ms_zb_plain_max) {
        handle_plain_dgram(rx, ip, port, dgram, len);
    }
    else if(dgram[0] >= ms_zb_enc_min && dgram[0] <= ms_zb_enc_max) {
        handle_encrypted_dgram(rx, ip, port, dgram, len);
    }

}


static void the_fd_handler_write(struct sue_fd_handler *h)
{
    struct ms_udp_receiver *rx = h->userdata;
    struct ms_transmit_item *item;
    int r;

    log_msg(llv_debug2, "the_fd_handler_write called");

    item = fetch_item_to_transmit(rx->txq);
    if(!item)
        return;

    log_msg(llv_debug2, 
            "sending %i bytes to %s",
            item->len - item->offset,
            ipport2a(item->ip, item->port));

    r = send_to(rx->fdh.fd, item->ip, item->port,
                item->buf + item->offset,
                item->len - item->offset);
    if(r != 0) {
        /* TODO: idk what to do now... */
    }
    txq_item_sent(item);
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

    log_msg(llv_debug2, "the_fd_handler_read called");
    rc = recvfrom(rx->fdh.fd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &addr_len);
    if(rc == -1)
        log_perror(llv_alert, "the_fd_handler_read", "recvfrom");
    ip = htonl(addr.sin_addr.s_addr);
    port = htons(addr.sin_port);

    log_msg(llv_debug, "received %d bytes from %s", rc, ipport2a(ip, port));
    
    handle_incoming_dgram(rx, ip, port, buf, rc);
}


static void the_fd_handler(struct sue_fd_handler *h, int r, int w, int x)
{
    struct ms_udp_receiver *rx = h->userdata;

    log_msg(llv_debug2, "the_fd_handler called (%s)(%s)",
                       r ? "r" : "-", w ? "w" : "-");

    if(r)
        the_fd_handler_read(h);
    if(w)
        the_fd_handler_write(h);
    if(x)  /* WTF?! */
        log_msg(llv_debug, "the_fd_handler want exeption? WTF?");

    h->want_read = 1;
    h->want_write = txq_want_write(rx->txq);
    h->want_except = 0;
}

static void the_timeout_hdl(struct sue_timeout_handler *hdl)
{
    struct ms_udp_receiver *rx = hdl->userdata;
    /* todo: */
    static int t = 1;

    log_msg(llv_debug2, "the_timeout_hdl called");

#if 1
    /* TODO: UDALI NAHUI */
    if(t) {
        dbug_print_all_peers(rx->peers);
        if(rx->the_cfg->listen_port == 24881) {
            unsigned int ip;
            unsigned short port = 24880 ;
            str2ip(&ip, "127.0.0.1");
            struct ms_peer* peer = get_peer_record(rx->peers, ip, port, 0);
            if(!peer) {
                log_msg(llv_alert, "WTF?");
                exit(1);
            }
            peer_set_init_assoc(peer);
            peer_set_assoc_status(peer, as_none);
        }
        t = 0;
    }
#endif

    peers_timer_hook(rx->peers);

    rx->fdh.want_read = 1;
    rx->fdh.want_write = txq_want_write(rx->txq);
    rx->fdh.want_except = 0;

    sue_timeout_set_from_now(hdl, housekeeping_interval, 0);
    sue_sel_register_timeout(rx->the_selector, &rx->tmoh);
}

static void enlist_peer_conf(struct peer_conf** list, struct peer_conf* conf)
{
    conf->next = *list;
    *list = conf;
}

static void add_test_peers(struct ms_node_cfg* cfg, struct ms_udp_receiver* rx)
{
    unsigned char* node1 = rx->comctx.identity->node_id;
    struct peer_conf* conf1;
    struct peer_conf* conf2;

    conf1 = malloc(sizeof(*conf1));
    conf2 = malloc(sizeof(*conf2));

    memcpy(conf1->node_id, node1, node_id_size);
    str2ip(&conf1->ip, "127.0.0.1");
    strcpy(conf1->name, "bebra");
    conf1->port = 24880;
    enlist_peer_conf(&cfg->first_peer, conf1);

    // memcpy(conf2->node_id, node1, node_id_size);
    // str2ip(&conf2->ip, "127.0.0.1");
    // conf2->port = def_port2;
    // enlist_peer_conf(&cfg->first_peer, conf2);
}

struct ms_udp_receiver* make_udp_receiver(struct sue_event_selector* s, struct ms_node_cfg* cfg)
{
    struct ms_udp_receiver* rx;
    int r;
    rx = malloc(sizeof(*rx));

    rx->fdh.fd = -1;
    rx->fdh.want_read = 1;
    rx->fdh.want_write = 0;
    rx->fdh.want_except = 0;
    rx->fdh.userdata = rx;
    rx->fdh.handle_fd_event = &the_fd_handler;

    rx->tmoh.userdata = rx;
    rx->tmoh.handle_timeout = &the_timeout_hdl;

    rx->the_selector = s;
    rx->the_cfg = cfg;


    r = comctx_init(&rx->comctx);
    if(!r) {
        log_msg(llv_alert, "problems initializing ctypto comm ctx");
        free(rx);
        return NULL;
    }
    r = comctx_init_node(&rx->comctx, cfg->keys_dir);
    if(!r) {
        log_msg(llv_alert, "problems loading node id");
        free(rx);
        return NULL;
    }

    add_test_peers(cfg, rx);

    rx->kndb = load_kndb(cfg);

    rx->peers = make_peer_collection(rx, rx->the_cfg, &rx->comctx);
    rx->txq = make_transmit_queue(s);

    #if 1
    test_init_kndb(rx->kndb, rx->comctx.identity->node_id, rx->comctx.identity->master_public_key);
    #endif


    log_msg(llv_debug, "udp_receiver initialized", 
                 hexdata2a(rx->comctx.identity->node_id, node_id_size));
    log_msg_bald(llv_debug, "node id:      %s", 
                 hexdata2a(rx->comctx.identity->node_id, node_id_size));
    log_msg_bald(llv_debug, "node pub key: %s",
                 hexdata2a(rx->comctx.identity->master_public_key, public_key_size));
    log_msg_bald(llv_debug, "kex pub key:  %s",
                 hexdata2a(rx->comctx.kex_public, kex_public_size));

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

    struct sockaddr_in own_addr;
    int sockfd, ok;

    own_addr.sin_family = AF_INET;
    own_addr.sin_port = htons(rx->the_cfg->listen_port);
    own_addr.sin_addr.s_addr = htonl(rx->the_cfg->listen_ip);
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd == -1) {
        log_perror(llv_alert, "start_udp_receiver", "socket");
        return 0;
    }
    ok = bind(sockfd, (struct sockaddr*)&own_addr, sizeof(own_addr));
    if(ok == -1) {
        log_perror(llv_alert, "start_udp_receiver", "bind");
        return 0;
    }

    /* success */

    rx->fdh.fd = sockfd;
    rx->fdh.want_read = 1;
    sue_sel_register_fd(rx->the_selector, &rx->fdh);

    sue_timeout_set_from_now(&rx->tmoh, housekeeping_start_delay, 0);
    sue_sel_register_timeout(rx->the_selector, &rx->tmoh);

    return 1;
}

void handle_assoc_process(struct ms_udp_receiver *rx,
                                struct ms_peer *peer)
{
    int since_last_rx, since_last_tx;
    unsigned int ip;
    unsigned short port;
    int assoc_status;

    peer_get_idle(peer, &since_last_rx, &since_last_tx);
    peer_get_addr(peer, &ip, &port);

    assoc_status = peer_assoc_status(peer);
    log_msg(llv_debug2, 
            "peer %s since_last_rx/tx %d/%d (st: %s)",
            ipport2a(ip, port), 
            since_last_rx, since_last_tx,
            assoc_status_str(assoc_status));

    switch(assoc_status) {
    case as_not_desired:
    case as_gave_up:
        return;
    case as_none:
        if(!peer_should_init_assoc(peer))
            return;
        send_echo_request(rx, peer);
        peer_set_assoc_status(peer, as_echo_request_sent);
        return;
    case as_echo_request_sent:
        if(since_last_rx < since_last_tx || since_last_tx < min_retry_time)
            return;
        if(since_last_rx > min_reset_time) {
            log_msg(llv_debug, "resetting association  for %s",
                    ipport2a(ip, port));
            peer_set_assoc_status(peer, as_none);
            return;
        }
        send_echo_request(rx, peer);
        log_msg(llv_debug, "resending echo_request for %s",
                ipport2a(ip, port));
        return;
    case as_assoc_request_sent:
        if(since_last_rx < since_last_tx || since_last_tx < min_retry_time)
            return;
        if(since_last_rx > min_reset_time) {
            log_msg(llv_debug, "resetting association  for %s",
                    ipport2a(ip, port));
            peer_set_assoc_status(peer, as_none);
            return;
        }
        log_msg(llv_debug, "resending assoc_request for %s",
                ipport2a(ip, port));
        send_assoc_request(rx, peer);
        return;
    case as_assoc_fini_sent:
        if (since_last_rx < since_last_tx || since_last_tx < min_retry_time)
            return;
        if(since_last_rx > min_reset_time) {
            log_msg(llv_debug, "resetting association  for %s",
                    ipport2a(ip, port));
            peer_set_assoc_status(peer, as_none);
            return;
        }
        log_msg(llv_debug, "resending assoc_fini for %s",
                ipport2a(ip, port));
        send_assoc_fini(rx, peer);
        return;
    case as_established:
        if(since_last_rx > rx->the_cfg->peer_timeout) {
            log_msg(llv_info, "association with %s seems dead",
                    ipport2a(ip, port));
            peer_set_assoc_status(peer, as_none);
            return;
        }
        if(since_last_tx > rx->the_cfg->keepalive_interval)
            send_enc_keepalive(rx, peer);
    }
}

static void do_rx_report(struct ms_udp_receiver *rx,
                         report_callback f, void *userdata)
{
    log_msg(llv_debug, "do_rx_report called");
    f(userdata, "running ms node with id:%s",
                hexdata2a(rx->comctx.identity->node_id, node_id_size));
    peers_report(rx->peers, f, userdata);
}

void udp_receiver_report(struct ms_udp_receiver *rx)
{
    int level = llv_normal | llv_private;
    log_msg(llv_normal, "got SIGUSR1");
    do_rx_report(rx, report_to_log_cb, &level);
}
