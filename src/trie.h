#ifndef _MS_TRIE_H
#define _MS_TRIE_H

#include <stddef.h>

struct trie_node {
    struct trie_node *a[16];
    void* userdata;
};

struct trie {
    struct trie_node* root;
};

void trie_init(struct trie* t);
void trie_clear(struct trie* t);

void* trie_get_with_len(struct trie* t, const char* key, int* out_len);

void* trie_get(struct trie* t, const char* key);
void** trie_provide(struct trie* t, const char* key);
int trie_delete(struct trie* t, const char *key);

#endif /* MS_TRIE_H */
