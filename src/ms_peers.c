#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ms_peers.h"
#include "ms_rx.h"
#include "ms_comm_ctx.h"
#include "addrcol.h"
#include "addrport.h"
#include "log.h"
#include "hexdata.h"
#include "ms_nodecfg.h"
#include "keyutils.h"
#include "crypdf.h"
#include "unitable.h"


enum { nonce_max_gap = 50 };

struct ms_peer {
    struct ms_peer_collection *master;
    struct addr_item *the_item;

    unsigned int ip;
    unsigned short port;

    struct peer_conf *the_conf_by_id, *the_conf_by_ip;
    char configured; /* bool means it added to permpeers*/

    char init_assoc; /* bool */
    int assoc_status;

    unsigned char node_id[node_id_size];
    unsigned char remote_pubkey[public_key_size];/* remote sign key*/
    unsigned char last_nonce[8];

    unsigned char remote_kex_pub[kex_public_size];
    unsigned char decrypt_key[cipher_key_size];
    unsigned char encrypt_key[cipher_key_size];

    unsigned long long last_token;   /* token from echo reply */

    unsigned char last_assoc_cookie[8];     /* cookie used for assoc_req/assoc_fini excange */
    unsigned long long last_assoc_timemark; /* timemark of last valid accepted assoc_req */

    unsigned long long last_rx;
    unsigned long long last_tx;
};

struct peercoll_item {
    struct ms_peer *peer;
    struct peercoll_item *next;
};

struct ms_peer_collection {
    struct ms_node_cfg *the_conf;
    struct ms_udp_receiver *the_rx;
    struct crypto_comm_ctx* the_comctx;
    struct addr_collection cooldown, peers;
    struct peercoll_item *permpeer_first;
    unsigned long long count;
};


const char* assoc_status_str(int status)
{
    switch (status) {
    case as_none: return "none";
    case as_gave_up: return "gave_up";
    case as_not_desired: return "not_desired";
    case as_established: return "established";
    case as_echo_request_sent: return "echo_request_sent";
    case as_assoc_request_sent: return "assoc_request_sent";
    case as_assoc_fini_sent: return "assoc_fini_sent";
    default: return "unknown";
    }
}

void dbug_print_all_peers(struct ms_peer_collection* col) 
{
    /*
        TODO: make traverse function with callback for map
        AND MOVE ALL THIS SHIT THERE
    */
    int i;
    for(i = 0; i < col->peers.map.capacity; i++) {
        struct ut_node* node = col->peers.map.buckets[i];
        for(;node;node = node->next) {
            struct addr_item* item = node->userdata;
            struct ms_peer* peer = item->userdata;
            log_msg(llv_debug, "peer in collection %s",
                    ipport2a(peer->ip, peer->port));
        }
    }
}


void peers_timer_hook(struct ms_peer_collection* col)
{
    long long tm;
    int res;
    struct peercoll_item* item;
    struct addr_item* addri;


    tm = time(NULL);

    log_msg(llv_debug2, "peers_timer_hook called, tm=%lld", tm);

    res = addrcoll_update(&col->cooldown, tm);
    if(res)
        addrcoll_process(&col->cooldown);

    res = addrcoll_update(&col->peers, tm);
    if(res)
        addrcoll_process(&col->peers);

    for(item = col->permpeer_first; item; item = item->next) {
        struct ms_peer* peer = item->peer;

        if(peer->assoc_status != as_not_desired &&
            peer->assoc_status != as_gave_up
        ) {
            handle_assoc_process(col->the_rx, item->peer);
        }
    }
    addri = col->peers.first;
    for(; addri; addri = addri->next) {
        struct ms_peer* peer = addri->userdata;
        int as = peer_assoc_status(peer);
        int since_last_rx, since_last_tx;
        unsigned int ip;
        unsigned short port;

        if(!addri->timeout_hook) continue;
        peer_get_idle(peer, &since_last_rx, &since_last_tx);
        peer_get_addr(peer, &ip, &port);
        log_msg(llv_debug2, 
                "peer %s since_last_rx/tx %d/%d (st: %s)\n",
                ipport2a(ip, port), 
                since_last_rx, since_last_tx,
                assoc_status_str(as));
    }
}

int timemark_minutes(const struct ms_peer_collection* col)
{
    return (col->peers.starttime + (long long)col->peers.curtime) / 60;
}

unsigned long long timemark_sec(const struct ms_peer_collection* col)
{
    return (col->peers.starttime + (long long)col->peers.curtime);
}


int peer_check_update_nonce(struct ms_peer* peer, const unsigned char* nonce,
                     const char* caller_name)
{
    unsigned long long known_nonce, new_nonce;
    char noncestr[16];

    known_nonce = u64_from_little_endian(peer->last_nonce);
    new_nonce = u64_from_little_endian(nonce);

    hexdata2str(noncestr, nonce, 8);
    if(nonce[7] != 0x01) {
        log_msg(llv_alert,
                "%s: trying to set nonce %s for %s but nonce[0] != 0x01 !!!",
                caller_name,
                noncestr,
                peer_description(peer));
        return 0;
    }
    
    if(known_nonce == 0 || new_nonce > known_nonce) {
        memcpy(peer->last_nonce, nonce, sizeof(peer->last_nonce));
        log_msg(llv_debug, "%s: setting known_nonce %s for %s",
                caller_name, noncestr, peer_description(peer));
        return 1;
    }

    if(known_nonce == new_nonce ||
        (new_nonce < known_nonce && known_nonce-new_nonce > nonce_max_gap)
    ) {
        log_msg(llv_debug,
            "%s: nonce check failed for %s (known %llx, new %llx)",
            caller_name, peer_description(peer), known_nonce, new_nonce);
        return 0;
    }
 
    return 1;
}

static int peer_has_id(const struct ms_peer* peer)
{
    return !all_zeroes(peer->node_id, node_id_size);
}

static void mspeer_timeout_hook(struct addr_item* item)
{
    unsigned int ip;
    unsigned short port;
    addritem_getaddr(item, &ip, &port);
    log_msg(llv_info, "peer %s timed out", ipport2a(ip, port));
    addritem_remove(item);
}

static void mspeer_destruction_hook(struct addr_item* item)
{
    if(item->userdata) {
        struct ms_peer *p = item->userdata;
        log_msg(llv_debug, "removing peer %s",
                peer_description(p));
        ms_rx_peer_gone(p->master->the_rx, p);
        free(item->userdata);
    } else {
        unsigned int ip;
        unsigned short port;
        addritem_getaddr(item, &ip, &port);
        log_msg(llv_debug, "removing peer %s (?!)",
                ipport2a(ip, port));
    }
}

/* must be only constructor for ms_peer */
static void peer_init(struct ms_peer_collection* col, struct addr_item* item)
{
    struct ms_peer *peer;
    unsigned int ip;
    unsigned short port;

    if(item->userdata)
        return;

    addritem_getaddr(item, &ip, &port);

    peer = malloc(sizeof(*peer));
    memset(peer, 0, sizeof(*peer));
    peer->master = col;
    peer->the_item = item;
    peer->ip = ip;
    peer->port = port;
    peer->last_assoc_timemark = 0;
    peer->last_rx = -1;
    peer->last_tx = -1;
    peer->last_token = 0;
    peer->init_assoc = 0;
    peer->assoc_status = as_none;

    peer->configured = 0;

    item->userdata = peer;
    item->timeout_hook = mspeer_timeout_hook;
    item->destruction_hook = mspeer_destruction_hook;
}

int peer_ever_had_assoc(const struct ms_peer* peer)
{
    return !all_zeroes(peer->remote_pubkey, sizeof(peer->remote_pubkey));
}

static void find_termz(char **p)
{
    while(**p)
        (*p)++;
}

const char *peer_description(const struct ms_peer* peer)
{
    static char res[256];
    char *p = res;

    strcpy(p, ipport2a(peer->ip, peer->port));
    find_termz(&p);

    if(peer->the_conf_by_ip) {
        *p = ' ';
        p++;
        *p = '[';
        p++;
        strcpy(p, peer->the_conf_by_ip->name);
        find_termz(&p);
        *p = ']';
        p++;
    }

    if(peer_has_id(peer)) {
        *p = ' ';
        p++;
        *p = '(';
        p++;
        if(peer->assoc_status != as_established) {
            *p = peer_ever_had_assoc(peer) ? '~' : '?';
            p++;
            *p = ':';
            p++;
        }
        hexdata2str(p, peer->node_id, node_id_size);
        p += node_id_size*2;
        if(peer->the_conf_by_id) {
            *p = ' ';
            p++;
            *p = '[';
            p++;
            strcpy(p, peer->the_conf_by_id->name);
            find_termz(&p);
            *p = ']';
            p++;
        }
        *p = ')';
        p++;
    }

    *p = 0;
    return res;
}

void peer_get_addr(const struct ms_peer* peer, unsigned int* ip, unsigned short* port)
{
    if(ip) 
        *ip = peer->ip;
    if(port) 
        *port = peer->port;
}

void peer_set_token(struct ms_peer* peer, unsigned long long token)
{
    peer->last_token = token;
}

unsigned long long peer_token(struct ms_peer* peer)
{
    return peer->last_token;
}


void peer_set_cookie(struct ms_peer* peer, const unsigned char cookie[8])
{
    memcpy(peer->last_assoc_cookie, cookie, 8);
}

void peer_generate_new_cookie(struct ms_peer* peer)
{
    get_random(&peer->last_assoc_cookie, 8);
}

const unsigned char* peer_cookie(struct ms_peer* peer)
{
    return peer->last_assoc_cookie;
}

int peer_check_cookie(const struct ms_peer* peer, const unsigned char cookie [8])
{
    return 0 == memcmp(peer->last_assoc_cookie, cookie, 8);
}

const unsigned char* peer_id(struct ms_peer* peer)
{
    return peer->node_id;
}

const unsigned char* peer_encrypt_key(const struct ms_peer* peer)
{
    return peer->encrypt_key;
}

const unsigned char* peer_decrypt_key(const struct ms_peer* peer)
{
    return peer->decrypt_key;
}

void peer_set_last_tm(struct ms_peer* peer, unsigned long long ts)
{
    peer->last_assoc_timemark = ts;
}

unsigned long long peer_get_last_tm(const struct ms_peer* peer)
{
    return peer->last_assoc_timemark;
}


int peer_assoc_status(const struct ms_peer* peer)
{
    return peer->assoc_status;
}

int peer_should_init_assoc(const struct ms_peer* peer)
{
    return peer->init_assoc;
}

void peer_set_init_assoc(struct ms_peer* peer)
{
    peer->init_assoc = 1;
}

void peer_reset_init_assoc(struct ms_peer* peer)
{
    peer->init_assoc = 0;
}

void peer_set_assoc_status(struct ms_peer* peer, int status)
{
    peer->assoc_status = status;
}

void update_peer_last_rx(struct ms_peer* peer)
{
    peer->last_rx = peer->master->peers.curtime;
    if(peer->the_item)
        addritem_update(peer->the_item);
}

void update_peer_last_tx(struct ms_peer* peer)
{
    peer->last_tx = peer->master->peers.curtime;
    if(peer->the_item)
        addritem_update(peer->the_item);
}

void peer_get_idle(const struct ms_peer* peer,
                   int *since_last_rx, int *since_last_tx)
{
    int curtime = peer->master->peers.curtime;

    if(since_last_rx)
        *since_last_rx = curtime - peer->last_rx;
    if(since_last_tx)
        *since_last_tx = curtime - peer->last_tx;
}


int peer_kex_public_is_same(const struct ms_peer* peer,
                            const unsigned char* kex_public)
{
     return 0 == memcmp(peer->remote_kex_pub, kex_public, kex_public_size);
}


int peer_set_kex_public(struct ms_peer_collection* col, struct ms_peer* peer,
                        const unsigned char* kex_public, int signchecked)
{
    if(0 == memcmp(peer->remote_kex_pub, kex_public, kex_public_size))
        return 1;

    if(!signchecked && peer->assoc_status == as_established)
        return 0;

    memcpy(peer->remote_kex_pub, kex_public, kex_public_size);

    derive_cipher_keys(col->the_comctx->kex_secret,
                       col->the_comctx->kex_public, peer->remote_kex_pub,
                       peer->encrypt_key, peer->decrypt_key);

    memset(peer->last_nonce, 0, sizeof(peer->last_nonce));

    log_msg(llv_debug, "set kex pub %s for %s",
            hexdata2a(peer->remote_kex_pub, kex_public_size),
            ipport2a(peer->ip, peer->port));
    log_msg_bald(llv_debug, "our kex pub %s",
            hexdata2a(col->the_comctx->kex_public, kex_public_size));
    return 1;
}

static void enlist_permpeer(struct ms_peer_collection* col, struct ms_peer* peer)
{
    struct peercoll_item *tmp;
    tmp = malloc(sizeof(*tmp));
    tmp->peer = peer;
    tmp->next = col->permpeer_first;
    col->permpeer_first = tmp;
}

static void add_configured_peers(struct ms_peer_collection* col)
{
    struct peer_conf* conf;
    int peer_count = 0;
    for(conf = col->the_conf->first_peer; conf; conf = conf->next) {
        struct addr_item* item;
        struct ms_peer* peer;
        if(!peer_conf_has_ip(conf))
            continue;
        if(conf->ip == col->the_conf->listen_ip &&
            conf->port == col->the_conf->listen_port) {
            log_msg(llv_debug, "skipping peer config for ourself ?!");
            continue;
        }
        item = addrcoll_permadd(&col->peers, conf->ip, conf->port);
        if(!item->userdata)
            peer_init(col, item);
        peer = item->userdata;
        peer->the_conf_by_ip = conf;
        if(peer_conf_has_id(conf)) {
            peer->the_conf_by_id = conf;
            memcpy(peer->node_id, conf->node_id, node_id_size);
        }
        peer->init_assoc = 0;
        peer->assoc_status = as_not_desired;
        peer->configured = 1;
        enlist_permpeer(col, peer);
        if(conf->type == ptp_mynode) {
            peer->init_assoc = 1;
            peer->assoc_status = as_none;
        }

        peer_count++;
    }
    log_msg(llv_debug, "configured peers added (%d)", peer_count);
}

struct ms_peer_collection* make_peer_collection(struct ms_udp_receiver* rx, 
                                                struct ms_node_cfg* cfg,
                                                struct crypto_comm_ctx* comctx)
{
    struct ms_peer_collection* col;
    long long tm;
    col = malloc(sizeof(*col));
    col->the_conf = cfg;
    col->the_rx = rx;
    col->the_comctx = comctx;
    col->count = 0;

    tm = time(NULL);
    addrcoll_init(&col->cooldown, tm, cfg->cooldown_timeout);
    addrcoll_init(&col->peers, tm, cfg->peer_timeout);


    add_configured_peers(col);

    return col;
}

struct ms_peer* get_peer_record(struct ms_peer_collection* col,
                         unsigned int ip, unsigned short port, int add)
{
    struct addr_item *p;
    p = addrcoll_find(&col->peers, ip, port, add);
    if(!p)
        return NULL;
    if(!p->userdata) {    
        peer_init(col, p);
        log_msg(llv_info, "new peer %s", ipport2a(ip, port));
    }
    return p->userdata;
}

int node_id_is_same(const struct ms_peer* peer, const unsigned char* id)
{
    return !memcmp(peer->node_id, id, node_id_size);
}

int peer_set_identity(struct ms_peer* peer,
                      const unsigned char* node_id,
                      const unsigned char* pubkey)
{
    struct peer_conf *conf;
    unsigned int ip = peer->ip;
    unsigned short port = peer->port;

    log_msg(llv_debug,
            "peer_set_identity called for %s to set %s",
            peer_description(peer), hexdata2a(node_id, node_id_size));

    if(peer_has_id(peer) &&
        (0 != memcmp(peer->node_id, node_id, node_id_size))
    ) {
        char nid_str[node_id_size * 2 + 1];
        char cur_nid_str[node_id_size * 2 + 1];
        hexdata2str(nid_str, node_id, node_id_size);
        hexdata2str(cur_nid_str, peer->node_id, node_id_size);
        log_msg(llv_normal,
            "peer_set_identity: for peer %s: refusing to replace id (%s with %s)",
            ipport2a(ip, port), cur_nid_str, nid_str);
        return 0;
    }

    for(conf = peer->master->the_conf->first_peer; conf; conf = conf->next) {
        int have_ip, have_id, match_ip, match_id;

        have_ip = peer->ip != PEER_IP_UNDEF;
        match_ip = ip == conf->ip && (port == conf->port || conf->port == 0);
        have_id = peer_conf_has_id(conf);
        match_id = have_id && 0 == memcmp(node_id, conf->node_id, node_id_size);

        if(!match_ip && !match_id)
            continue;
        if(have_ip && match_ip && have_id && !match_id) {
            char nid_str[node_id_size * 2 + 1];
            char conf_nid_str[node_id_size * 2 + 1];
            hexdata2str(nid_str, node_id, node_id_size);
            hexdata2str(conf_nid_str, conf->node_id, node_id_size);
            log_msg(llv_normal,
                "refusing assoc with %s: %s mismatches our config %s",
                ipport2a(ip, port), nid_str, conf_nid_str);
            return 0;
        }
        if(have_ip && match_ip && !peer->the_conf_by_ip) {
            peer->the_conf_by_ip = conf;
        }
        if(have_id && match_id && !peer->the_conf_by_id) {
            peer->the_conf_by_id = conf;
        }
    }

    memcpy(peer->node_id, node_id, node_id_size);
    if(pubkey)
        memcpy(peer->remote_pubkey, pubkey, public_key_size);
    return 1;
}

static const char *decimal2a(unsigned int n)
{
    static char res[16];
    char *p = res + (sizeof(res)-1);

    if(n == 0)
        return "0";

    *p = 0;
    while(n > 0) {
        p--;
        *p = '0' + n % 10;
        n /= 10;
    }

    return p;
}

static void
single_peer_report(struct ms_peer *peer, long t, report_callback cb, void *ud)
{
    cb(ud, "[%s] %s (%s)", 
       t == -1 ? "-" : decimal2a(t),
       peer_description(peer),
       assoc_status_str(peer->assoc_status));
}

void peers_report(struct ms_peer_collection *col, report_callback f, void *ud)
{
    struct peercoll_item *p;
    struct addr_item *ia;

    for(p = col->permpeer_first; p; p = p->next) {
        struct ms_peer *fp = p->peer;
        single_peer_report(fp, -1, f, ud);
    }
    for(ia = col->peers.first; ia; ia = ia->next) {
        struct ms_peer *fp = ia->userdata;
        single_peer_report(fp, col->peers.curtime - ia->timemark, f, ud);
    }
}



