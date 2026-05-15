#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ms_nodeid.h"
#include "log.h"
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

int load_nodeid_file(struct ms_nodeid_file* idf, const char* fname)
{
    FILE* f;
    struct stat st;
    unsigned char secret[node_secret_size];
    size_t r;
    int res;

    res = stat(fname, &st);
    if (res != 0) {
        log_perror(llv_alert, "load_nodeid_file", "unable to find id file");
        return 1;
    }

    if(st.st_size != node_secret_size) {
        log_msg(llv_alert, "invalid id file");
        log_msg_bald(llv_alert, "size: %llu", st.st_size);
        return 1;
    }

    f = fopen(fname, "rb");
    if(!f) {
        log_perror(llv_alert, "load_nodeid_file", "unable to open id file");
        return 1;
    }

    r = fread(secret, 1, node_secret_size, f);
    if (r != node_secret_size) {
        crypto_wipe(secret, sizeof(secret));
        log_msg(llv_alert, "[FATAL] seed is too short");
        return 1;
    }

    fclose(f);

    crypto_eddsa_key_pair(idf->master_secret_key, 
                          idf->master_public_key,
                          secret);

    memcpy(idf->node_id, 
           idf->master_public_key + public_key_size - node_id_size,
           node_id_size);
    
    crypto_wipe(secret, node_secret_size);
    return 0;
}


void dispose_nodeid(struct ms_nodeid_file* ni)
{
    crypto_wipe(ni, sizeof(*ni));
    free(ni);
}

