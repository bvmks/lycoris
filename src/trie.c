#include <stdlib.h>
#include <string.h>
#include "trie.h"

static struct trie_node* make_node() 
{
    struct trie_node* res = malloc(sizeof(*res));
    memset(res->a, 0, sizeof(res->a));
    res->userdata = NULL;
    return res;
}

static void traverse_clear(struct trie_node *node) 
{
    int i;
    if (!node)
        return;
    for (i = 0; i < 16; i++) {
        if (node->a[i]) {
            traverse_clear(node->a[i]);
        }
    }
    free(node);
}

void trie_init(struct trie* t) 
{
    t->root = NULL;
}

void trie_clear(struct trie* t) 
{
    if (!t->root) 
        return;
    traverse_clear(t->root);
    t->root = NULL;
}

void* trie_get(struct trie* t, const char* key) 
{

}

void** trie_provide(struct trie* t, const char* key) 
{

}

int trie_delete(struct trie* t, const char *key) 
{

}
