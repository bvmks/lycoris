#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ms_nodeid.h"
#include "../message.h"
#include "../../lib/monocypher/monocypher.h"


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

    nodeid_init(ni);

    f = fopen(fname, "rb");
    if(!f) {
        message_perror(mlv_alert, "FATAL", "unable to open id file");
        return 1;
    }

    r = fread(ni->secret, 1, node_secret_size, f);
    fclose(f);

    if (r != node_secret_size) {
        memset(ni->secret, 0, sizeof(ni->secret));
        message_perror(mlv_alert, "FATAL", "seed file is too short or corrupted");
        nodeid_init(ni);
        return 1;
    }

    crypto_eddsa_key_pair(ni->master_privat_key, ni->master_public_key, ni->secret);

    memcpy(ni->node_id, ni->master_public_key + (public_key_size - node_id_size), node_id_size);

    memset(ni->secret, 0, node_secret_size);

    return 0;
}

void dispose_nodeid(struct ms_nodeid_file* ni)
{
    memset(ni, 0, sizeof(*ni));
    free(ni);
}

