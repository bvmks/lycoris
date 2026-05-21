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

enum trie_iter_res {
    trie_iter_need_more,
    trie_iter_found,
    trie_iter_not_found
};

struct trie_iterator {
    struct trie_node *current;
    int half;
};

void trie_iterator_reset(struct trie_iterator *it, struct trie *t);

int trie_iterator_feed(struct trie_iterator *it, char c);

void* trie_iterator_get_data(const struct trie_iterator *it);

#endif /* MS_TRIE_H */
