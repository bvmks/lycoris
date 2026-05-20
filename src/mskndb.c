#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

#include "hexdata.h"
#include "ms_kndb.h"
#include "fileutil.h"

#include "_version.h"

struct cmdline_args {
    char* kndb_dir;
    int help;
    char errmsg[80];
    char* command;
    int cmd_arg_idx;
};

static void set_def_args(struct cmdline_args* args) 
{
    args->kndb_dir = NULL;
    args->help = 0;
    args->errmsg[0] = 0;
    args->command = NULL;
    args->cmd_arg_idx = 0;
}


static void print_help()
{
    fputs(
        "mskndb\n"
        "vers. " MS_VERSION "\n"
        "\n"
        "usage: \"mskndb [options] <command> [command_args]\"\n"
        "options:\n"
        "    -d <db_dir>      default is $HOME/.ms/keys/kndb\n"
        "    -h               show this text\n"
        "commands:\n"
        "    list             list all known nodes\n"
        "    add <id> <key>   add node to db (arguments must be hex strings)\n"
        "\n",
        stdout);
}

static void print_need_param(char c, struct cmdline_args *args)
{
    sprintf(args->errmsg, "option -%c needs a parameter\n", c);
}

static int
parse_cmdline(int argc, char **argv, struct cmdline_args *args)
{
    int idx = 1;
    while(idx < argc) {
        if(argv[idx][0] == '-') {
            switch(argv[idx][1]) {
            case 'd':
                if(idx+1 >= argc || argv[idx+1][0] == '-') {
                    print_need_param('c', args);
                    return 0;
                }
                args->kndb_dir = (char*)argv[idx+1];
                idx += 2;
                break;
            case 'h':
                args->help = 1;
                idx++;
                break;
            default:
                sprintf(args->errmsg, "unknown option '-%c'\n", argv[idx][1]);
                return 0;
            }
        } else {
            args->command = argv[idx];
            args->cmd_arg_idx = idx;
            break;
        }
    }
    return 1;
}

static void settle_kndb_path(struct cmdline_args *args)
{
    settle_localpath(&args->kndb_dir, ".ms/kndb");
}

static void
process_cmdline(int argc, char **argv, struct cmdline_args *args)
{
    int res;

    set_def_args(args);
    res = parse_cmdline(argc, argv, args);
    if(!res) {
        printf("FATAL: wrong args: %s\n", args->errmsg);
        exit(1);
    }
    if(args->help) {
        print_help();
        exit(0);
    }

    if(!args->kndb_dir)
        settle_kndb_path(args);
}

int kndb_list_nodes(struct known_nodes_db* db)
{
    DIR *dir;
    struct dirent *entry;

    if (!db || !db->dir) {
        printf("kndb_list_nodes: database directory not set\n");
        return kndb_res_file_error;
    }

    dir = opendir(db->dir);
    if (!dir) {
        if(errno != ENOENT)
            printf("kndb_list_nodes: failed to open dir %s\n", db->dir);
        else {
            printf("kndb_list_nodes: dir not found %s\n", db->dir);
        }
        return kndb_res_file_error;
    }

    printf("Known nodes in database (%s):\n", db->dir);
    int count = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        printf("  Node ID: %s\n", entry->d_name);
        count++;
    }
    if (count == 0) {
        printf("  <database is empty>\n");
    }
    closedir(dir);
    return kndb_res_success;
}

int main(int argc, char **argv)
{
    struct cmdline_args args;
    struct known_nodes_db* db;

    process_cmdline(argc, argv, &args);

    if (args.help || argc == 1) {
        print_help();
        return 0;
    }

    if (!args.command) {
        fprintf(stderr, "FATAL: No command specified\n");
        return 1;
    }

    settle_kndb_path(&args);

    db = make_kndb(args.kndb_dir);

    if (strcmp(args.command, "list") == 0) {
        kndb_list_nodes(db);
    } 
    else if (strcmp(args.command, "add") == 0) {
        char* hex_id;
        char* hex_key;
        unsigned char bin_id[node_id_size];
        unsigned char bin_key[public_key_size];

        if (args.cmd_arg_idx + 2 >= argc) {
            fprintf(stderr, "FATAL: Command 'add' requires 2 arguments: <node_id> and <pubkey>\n");
            return 1;
        }

        hex_id = argv[args.cmd_arg_idx + 1];
        hex_key = argv[args.cmd_arg_idx + 2];
        
        if(hexstr2data(bin_id, node_id_size, hex_id) != node_id_size) {
            fprintf(stderr, "FATAL: Invalid node id length (must be %d hex characters)\n", node_id_size * 2);
            return 1;
        }
        if(hexstr2data(bin_key, public_key_size, hex_key) != public_key_size) {
            fprintf(stderr, "FATAL: Invalid pubkey length (must be %d hex characters)\n", public_key_size * 2);
            return 1;
        }
        
        int res = kndb_save_node(db, bin_id, bin_key);
        if (res == kndb_res_success) {
            printf("SUCCESS: Node %s successfully added to database.\n", hex_id);
        } else {
            fprintf(stderr, "ERROR: Failed to save node to database (code: %d)\n", res);
        }
    } 
    else {
        fprintf(stderr, "FATAL: Unknown command '%s'\n", args.command);
        print_help();
    }
    return 0;
}
