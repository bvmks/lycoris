#ifndef _STUPID_PARSER_H
#define _STUPID_PARSER_H

struct peer_conf;

struct peer_conf* parse_peers_file(const char* filename);

void free_peer_conf_list(struct peer_conf* list);

#endif /* _PEER_PARSER_H */
