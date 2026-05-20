#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "ms_nodecfg.h"
#include "addrport.h"
#include "stupid_peers_parser.h"
#include "hexdata.h"
#include "log.h"

void free_peer_conf_list(struct peer_conf* list)
{
    while (list) {
        struct peer_conf* tmp = list->next;
        free(list);
        list = tmp;
    }
}

static struct peer_conf* make_peer()
{
    struct peer_conf* p = malloc(sizeof(*p));
    memset(p, 0, sizeof(*p));
    p->type = mspt_undef;
    return p;
}

struct peer_conf* parse_peers_file(const char* filename)
{
    FILE* f;
    struct peer_conf* head = NULL;
    struct peer_conf* current = NULL;
    char line[512];
    int line_num = 0;
        
    f = fopen(filename, "r");
        
    if (!f) {
        log_msg(llv_alert, "peer_parser: cannot open peers config file %s", filename);
        return NULL;
    }

    while (fgets(line, sizeof(line), f)) {
        char key[128];
        char val[384];
        int tokens;
            
        line_num++;
        line[strcspn(line, "\r\n")] = '\0';
        tokens= sscanf(line, "%127s %383s", key, val);
        
        if (tokens <= 0 || key[0] == '#') {
            continue;
        }

        if (strcmp(key, "peer") == 0) {
            struct peer_conf* new_peer = make_peer();
            if (!new_peer) {
                log_msg(llv_alert, "peer_parser: out of memory at line %d", line_num);
                free_peer_conf_list(head);
                fclose(f);
                return NULL;
            }

            if (!head) {
                head = new_peer;
            } else {
                current->next = new_peer;
            }
            current = new_peer;
            continue;
        }

        if (!current) {
            log_msg(llv_alert, "peer_parser: configuration parameter '%s' before 'peer' marker at line %d", key, line_num);
            continue; 
        }

        if (strcmp(key, "name") == 0) {
            if (tokens < 2) {
                log_msg(llv_alert, "peer_parser: missing value for 'name' at line %d", line_num);
                continue;
            }
            strncpy(current->name, val, peer_name_length_limit);
            current->name[peer_name_length_limit] = '\0';
        } 
        else if (strcmp(key, "id") == 0) {
            if (tokens < 2) {
                log_msg(llv_alert, "peer_parser: missing value for 'id' at line %d", line_num);
                continue;
            }
            if (strlen(val) != node_id_size * 2) {
                log_msg(llv_alert, "peer_parser: invalid hex ID length at line %d (expected %d chars)", line_num, node_id_size * 2);
                continue;
            }
            hexstr2data(current->node_id, node_id_size, val);
        } 
        else if (strcmp(key, "type") == 0) {
            if (tokens < 2) {
                log_msg(llv_alert, "peer_parser: missing value for 'type' at line %d", line_num);
                continue;
            }
            if (strcmp(val, "peer") == 0) current->type = mspt_peer;
            else if (strcmp(val, "server") == 0) current->type = mspt_server;
            else {
                log_msg(llv_alert, "peer_parser: unknown peer type '%s' at line %d", val, line_num);
            }
        } 
        else if (strcmp(key, "addr") == 0) {
            if (tokens < 2) {
                log_msg(llv_alert, "peer_parser: missing value for 'addr' at line %d", line_num);
                continue;
            }
            if(!str2ipport(&current->ip, &current->port, val)) {
                log_msg(llv_alert, "peer_parser: invalid address '%s' at line %d", val, line_num);
            }
        } 
        else {
            log_msg(llv_alert, "peer_parser: unknown configuration key '%s' at line %d", key, line_num);
        }
    }

    fclose(f);
    return head;
}
