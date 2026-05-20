#define _XOPEN_SOURCE 500      /* for vsnprintf */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "crypdf.h"
#include "ms_kndb.h"
#include "hexdata.h"
#include "log.h"
#include "ms_nodecfg.h"
#include "fileutil.h"


struct known_nodes_db* make_kndb(char* dir)
{
    struct known_nodes_db* res;
    res = malloc(sizeof(*res));
    res->dir = dir;
    return res;
}

static void mk_knnode_paths(const char *wdir, const unsigned char node_id[], struct knnode_paths* path)
{
    char nodename[node_id_size * 2 + 1];
    char nodepath[8 + node_id_size * 2 + 1];  /* 8 is for prefix */

    hexdata2str(nodename, node_id, node_id_size);

    nodepath[0] = nodename[0];
    nodepath[1] = nodename[1];
    nodepath[2] = nodename[2];
    nodepath[3] = '/';
    nodepath[4] = nodename[3];
    nodepath[5] = nodename[4];
    nodepath[6] = nodename[5];
    nodepath[7] = '/';
    strcpy(nodepath + 8, nodename);

    path->node_dir = concat_path(wdir, nodepath);
    path->node_file = concat_path(path->node_dir, "node");
}

static void knnode_paths_cleanup(struct knnode_paths* path)
{
    if(path->node_file)
        free(path->node_file);
    if(path->node_dir)
        free(path->node_dir);
}

enum knnode_parse_res {
    knparse_res_ok = 0,

    knparse_res_file_not_found = 1,
    knparse_res_cant_open_file,
    knparse_res_invalid_file,
    knparse_res_unexpected,
};

static int parse_node(struct knnode_paths* path, struct ms_known_node* node)
{
    FILE *f;
    char* res;
    char buf[1024];

    f = fopen(path->node_file, "r");
    if(!f)
        return errno == ENOENT ?
            knparse_res_file_not_found : knparse_res_cant_open_file;

    res = fgets(buf, sizeof(buf), f);
    if(!res) {
        if (errno != 0) {
            log_perror(llv_alert, "parse_node", "fgets");
            return knparse_res_unexpected;
        }
    }
    if(hexstr2data(node->id, node_id_size, buf) != node_id_size) {
        log_msg(llv_alert, "invalid node file %s", path->node_file);
        return knparse_res_invalid_file;
    }

    res = fgets(buf, sizeof(buf), f);
    if(!res) {
        if (errno != 0) {
            log_perror(llv_alert, "parse_node", "fgets");
            return knparse_res_unexpected;
        }
    }
    if(hexstr2data(node->pubkey, public_key_size, buf) != public_key_size) {
        log_msg(llv_alert, "invalid node file %s", path->node_file);
        return knparse_res_invalid_file;
    }

    fclose(f);

    return knparse_res_ok;
}

static int kparse_res2kndb_res(int kparse)
{
    switch (kparse) {
    case knparse_res_ok: return kndb_res_success;

    case knparse_res_invalid_file:
    case knparse_res_cant_open_file: return kndb_res_file_error;
    default:
    case knparse_res_unexpected:     return kndb_res_unexpected;
    case knparse_res_file_not_found: return kndb_res_node_unknown;
    }
}

int kndb_get_node(struct known_nodes_db* db,
                  const unsigned char* node_id,
                  unsigned char* pubkey)
{
    struct knnode_paths f;
    struct ms_known_node node;
    int res;
    
    mk_knnode_paths(db->dir, node_id, &f);

    res = parse_node(&f, &node);
    if(res == knparse_res_file_not_found) {
        res = kndb_res_node_unknown;
        goto quit;
    }
    if(res != knparse_res_ok) {
        res = kparse_res2kndb_res(res);
        goto quit;
    }
    res = kndb_res_success;
quit:
    knnode_paths_cleanup(&f);
    return res;
}

static int serialize_node(struct knnode_paths* path, struct ms_known_node* node) {
    FILE *f;
    int res;
    char datastr[1024];

    f = fopen(path->node_file, "w");
    if(!f)
        return knparse_res_cant_open_file;

    sprintf(datastr , "%s\n", hexdata2a(node->id, node_id_size));
    res = fputs(datastr, f);
    if(!res) {
        log_perror(llv_alert, "parse_node", "fgets");
        return knparse_res_unexpected;
    }

    sprintf(datastr , "%s\n", hexdata2a(node->pubkey, public_key_size));
    res = fputs(datastr, f);
    if(!res) {
        log_perror(llv_alert, "parse_node", "fgets");
        return knparse_res_unexpected;
    }

    fclose(f);

    return knparse_res_ok;
}

int kndb_save_node(struct known_nodes_db* db,
                   const unsigned char *node_id,
                   unsigned char *pubkey)
{
    struct knnode_paths f;
    struct ms_known_node node;
    int res;
    
    mk_knnode_paths(db->dir, node_id, &f);
    res = make_directory_path(f.node_dir, 0);
    if(res == -1) {
        res = kndb_res_file_error;
        goto quit;
    }

    memcpy(node.id, node_id, node_id_size);
    memcpy(node.pubkey, pubkey, public_key_size);

    res = serialize_node(&f, &node);
    if(res != knparse_res_ok) {
        res = kparse_res2kndb_res(res);
        goto quit;
    }
    res = kndb_res_success;
quit:
    knnode_paths_cleanup(&f);
    return res;
}
