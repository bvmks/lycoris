#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ms_nodeid.h"
#include "ms_nodecfg.h"
#include "message.h"
#include "fileutil.h"
#include "../lib/monocypher/monocypher.h"


void nodeid_init(struct ms_nodeid_file* ni)
{
    memset(ni, 0, sizeof(struct ms_nodeid_file));
}

struct ms_nodeid_file* make_nodeid()
{
    struct ms_nodeid_file* ni;
    ni = malloc(sizeof(*ni));
    nodeid_init(ni);
    return ni;
}

int load_nodeid_file(struct ms_nodeid_file* ni, const char* fname)
{
    FILE* f;
    size_t r;
    unsigned char pub_key_hash[32];

    nodeid_init(ni);

    f = fopen(fname, "rb");
    if(!f) {
        message_perror(mlv_alert, "FATAL", "unable to open id file");
        return 1;
    }

    r = fread(ni->secret, 1, node_secret_size, f);
    fclose(f);

    if (r != node_secret_size) {
        crypto_wipe(ni->secret, sizeof(ni->secret));
        message_perror(mlv_alert, "FATAL", "seed file is too short or corrupted");
        nodeid_init(ni);
        return 1;
    }

    crypto_eddsa_key_pair(ni->master_privat_key, ni->master_public_key, ni->secret);

    crypto_blake2b(pub_key_hash, sizeof(pub_key_hash), ni->master_public_key, public_key_size);
    memcpy(ni->node_id, pub_key_hash, node_id_size);
    
    crypto_wipe(ni->secret, node_secret_size);
    return 0;
}

struct ms_nodeid_file* load_node_id(struct ms_node_cfg *cfg)
{
    struct ms_nodeid_file* id;
    char* keyfile;
    int lfail;

    id = malloc(sizeof(*id));
    keyfile = concat_path(cfg->keys_dir, "id");
    lfail = load_nodeid_file(id, keyfile);
    free(keyfile);
    if(lfail) {
        dispose_nodeid(id);
        free(id);
        return NULL;
    }
    return id;
}

void dispose_nodeid(struct ms_nodeid_file* ni)
{
    crypto_wipe(ni, sizeof(*ni));
    free(ni);
}

