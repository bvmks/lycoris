#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ms_peers.h"
#include "ms_rx.h"
#include "ms_comm_ctx.h"
#include "ms_nonce.h"
#include "addrcol.h"
#include "addrport.h"
#include "message.h"
#include "hexdata.h"

void peers_timer_hook(struct ms_peer_collection *coll)
{
    long long tm;
    int res;
    struct peercoll_item *item;

    tm = time(NULL);

    message(mlv_debug2, "[DEBUG] ms_peercoll_hook called, tm=%lld", tm);

    res = addrcoll_update(&coll->cooldown, tm);
    if(res)
        addrcoll_process(&coll->cooldown);

    res = addrcoll_update(&coll->peers, tm);
    if(res)
        addrcoll_process(&coll->peers);

    for(item = coll->permpeer_first; item; item = item->next) {
        struct ms_peer *fp = item->peer;
        if(fp->assoc_status != msas_not_needed &&
            fp->assoc_status != msas_gave_up
        ) {
            handle_association_process(coll->the_rx, item->peer);
        }
    }
}

static void mspeer_timeout_hook(struct addr_item *item)
{
    unsigned int ip;
    unsigned short port;
    addritem_getaddr(item, &ip, &port);
    message(mlv_info, "[INFO] peer %s timed out\n", ipport2a(ip, port));
    inaddritem_remove(item);
}

static void mspeer_destruction_hook(struct addr_item *item)
{
    if(item->userdata) {
        struct ms_peer *p = item->userdata;
        message(mlv_debug, "[DEBUG] removing peer %s",
                ms_peer_description(p));
        ms_rx_peer_gone(p->the_master->the_rx, p);
        free(item->userdata);
    } else {
        unsigned int ip;
        unsigned short port;
        addritem_getaddr(item, &ip, &port);
        message(mlv_debug, "[DEBUG] removing peer %s (?!)",
                ipport2a(ip, port));
    }
}

/* must be only constructor for ms_peer */
static void ms_peer_init(struct ms_peer_collection* coll, struct addr_item* item)
{
    struct ms_peer *p;
    unsigned int ip;
    unsigned short port;

    if(item->userdata)
        return;

    addritem_getaddr(item, &ip, &port);

    p = malloc(sizeof(*p));
    memset(p, 0, sizeof(*p));
    p->the_master = coll;
    p->the_item = item;
    p->ip = ip;
    p->port = port;
    p->created_at = 0;
    p->last_rx = -1;
    p->last_tx = -1;
    p->last_cookie = -1;
    p->init_assoc = 0;
    p->assoc_status = msas_not_needed;
    comctx_init(&p->comctx);
    item->userdata = p;
    item->timeout_hook = mspeer_timeout_hook;
    item->destruction_hook = mspeer_destruction_hook;
}


void ms_peer_fill_nounce(struct ms_peer* p, unsigned char* n)
{
    fill_nounce(&p->comctx.nonce, n);
}


const char* ms_peer_description(const struct ms_peer* p)
{
    /*TODO*/
    return 0;
}

void ms_peer_getaddr(const struct ms_peer* p, unsigned int* ip, unsigned short* port)
{
    if(ip) 
        *ip = p->ip;
    if(port) 
        *port = p->port;
}

void ms_peer_set_cookie(struct ms_peer* p, unsigned long long cookie)
{
    p->last_cookie = cookie;
}

unsigned long long ms_peer_get_cookie(struct ms_peer* p)
{
    return p->last_cookie;
}

const unsigned char* ms_peer_get_id(struct ms_peer* p)
{
    return p->id;
}

const unsigned char* ms_peer_get_kex(struct ms_peer* p)
{
    return p->comctx.kex_public;
}


int ms_peer_assoc_status(const struct ms_peer* p)
{
    return p->assoc_status;
}

void ms_peer_set_assoc_status(struct ms_peer* p, int status)
{
    p->assoc_status = status;
}

void update_peer_last_rx(struct ms_peer *fp)
{
    fp->last_rx = fp->the_master->peers.curtime;
}

void update_peer_last_tx(struct ms_peer *fp)
{
    fp->last_tx = fp->the_master->peers.curtime;
}

int peer_kex_public_is_same(const struct ms_peer *fp,
                            const unsigned char *kex_public)
{
     return 0 == memcmp(fp->comctx.remote_kex_public, kex_public, kex_public_size);
}

int peer_set_kex_public(struct ms_peer_collection *coll, struct ms_peer *peer,
                        const unsigned char *kex_public, int signchecked)
{
    if(0 == memcmp(peer->comctx.remote_kex_public, kex_public, kex_public_size))
        return 1;    /* nothing new */

    if(!signchecked && peer->assoc_status == msas_established)
        return 0;    /* this means established cryptographic association */

    memcpy(peer->comctx.remote_kex_public, kex_public, kex_public_size);

    derive_keys(peer->comctx.kex_secret,
                peer->comctx.kex_public, peer->comctx.remote_kex_public,
                peer->comctx.encrypt_key, peer->comctx.decrypt_key);

    ms_nonce_init(&peer->comctx.nonce);

    message(mlv_debug, "[DEBUG] set kex pub %s for %s",
            hexdata2a(peer->comctx.remote_kex_public, kex_public_size),
            ipport2a(peer->ip, peer->port));
    return 1;
}


struct ms_peer_collection* make_peer_collection(struct ms_udp_receiver* node, struct ms_node_cfg* cfg)
{
    struct ms_peer_collection* col;
    col = malloc(sizeof(*col));
    col->count = 0;
    return col;
}

struct ms_peer* get_peer_record(struct ms_peer_collection* coll,
                         unsigned int ip, unsigned short port, int add)
{
    struct addr_item *p;
    p = addrcoll_find(&coll->peers, ip, port, add);
    if(!p)
        return NULL;
    if(!p->userdata) {    
        ms_peer_init(coll, p);
        message(mlv_info, "[INFO] new peer %s\n", ipport2a(ip, port));
    }
    /* !!! the following block may be removed at any time, it's debug only */
    return p->userdata;
}


