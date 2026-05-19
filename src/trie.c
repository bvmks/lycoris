#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "trie.h"
#include "utils.h"

static struct trie_node* make_node() 
{
    struct trie_node* res = malloc(sizeof(*res));
    memset(res, 0, sizeof(*res));
    return res;
}

static void traverse_clear(struct trie_node* node) 
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

static void** traverse(struct trie_node* n, const char* key, int half, int add)
{
    struct trie_node** tmp;
    int idx;

    if(is_term(*key))
        return &n->userdata;

    idx = (half ? *key : (*key >> 4)) & 0x0f;
    tmp = &(n->a[idx]);

    if(!*tmp && !add) {
        return NULL;
    }
    if(!*tmp)
        *tmp = make_node();
    return traverse(*tmp, half ? key+1 : key, !half, add);
}

static int has_children(struct trie_node* n) 
{
    int i;
    for (i = 0; i < 16; i++) {
        if (n->a[i]) return 1;
    }
    return 0;
}

static int traverse_delete(struct trie_node* n, const char* key, int half)
{
    int idx;
    struct trie_node* tmp;

    if (!n) return 0;


    if (is_term(*key)) {
        n->userdata = NULL;
        if(has_children(n))
            return 0;
        return 1;
    }

    idx = (half ? *key : (*key >> 4)) & 0x0f;
    tmp = n->a[idx];

    if (!tmp) return 0;

    if (traverse_delete(tmp, half ? key + 1 : key, !half)) {
        free(tmp);
        n->a[idx] = NULL;

        if (n->userdata) return 0;
        if(has_children(n))
            return 0;
        return 1;
    }
    return 0;
}


/* interface */

void trie_init(struct trie* t) 
{
    t->root = make_node();
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
    void** res;
    res = traverse(t->root, key, 0, 0);
    if(!res)
        return NULL;
    return *res ? *res : NULL;
}

void** trie_provide(struct trie* t, const char* key) 
{
    if(!t->root)
        t->root = make_node();
    return traverse(t->root, key, 0, 1);
}

int trie_delete(struct trie* t, const char *key) 
{
    if (is_term(*key)) return 0;
    return traverse_delete(t->root, key, 0);
}

#ifdef _TEST_TRIE

void cmd_get(void)
{ 
    printf("  -> cmd GET\n"); 
}
void cmd_goto(void)
{ 
    printf("  -> cmd GOTO\n"); 
}

void cmd_post(void)
{
    printf("  -> cmd POST\n"); 
}

void cmd_status(void)
{
    printf("  -> cmd STATUS\n"); 
}

int main() {
    struct trie t;
    void** slot;
    void* res;

    printf("running trie tests...\n");

    trie_init(&t);

    printf("[TEST] Adding commands...\n");
    
    slot = trie_provide(&t, "GET");
    assert(slot != NULL);
    *slot = (void*)cmd_get;

    slot = trie_provide(&t, "GOTO");
    assert(slot != NULL);
    *slot = (void*)cmd_goto;

    slot = trie_provide(&t, "POST");
    assert(slot != NULL);
    *slot = (void*)cmd_post;

    printf("[TEST] Searching for exact matches...\n");
    res = trie_get(&t, "GET");
    assert(res == (void*)cmd_get);
    
    res = trie_get(&t, "GOTO");
    assert(res == (void*)cmd_goto);
    
    res = trie_get(&t, "POST");
    assert(res == (void*)cmd_post);

    printf("[SUCCESS]\n");

    printf("[TEST] Searching for non-existent keys...\n");
    res = trie_get(&t, "GE");
    assert(res == NULL);
    
    res = trie_get(&t, "GETS");
    assert(res == NULL);
    
    res = trie_get(&t, "UNKNOWN");
    assert(res == NULL);
    printf("[SUCCESS]\n");

    printf("[TEST] Testing string terminators...\n");
    res = trie_get(&t, "GET /index.html");
    assert(res == (void*)cmd_get);

    res = trie_get(&t, "POST\n");
    assert(res == (void*)cmd_post);

    printf("[SUCCESS]\n");

    printf("[TEST] Overwriting userdata in an existing node...\n");
    slot = trie_provide(&t, "GET");
    *slot = (void*)cmd_status;
    
    res = trie_get(&t, "GET");
    assert(res == (void*)cmd_status);
    
    slot = trie_provide(&t, "GET");
    *slot = (void*)cmd_get;
    printf("[SUCCESS]\n");

    printf("[TEST] Testing element deletion...\n");
    int del_res = trie_delete(&t, "GOTO");
    assert(del_res == 0 || del_res == 1);
    
    res = trie_get(&t, "GOTO");
    assert(res == NULL);
    
    res = trie_get(&t, "GET");
    assert(res == (void*)cmd_get);

    printf("[SUCCESS]\n");

    trie_delete(&t, "GET");
    res = trie_get(&t, "GET");
    assert(res == NULL);
    printf("[SUCCESS]\n");

    printf("[TEST] Full trie clearing...\n");
    trie_clear(&t);
    assert(t.root == NULL);
    
    slot = trie_provide(&t, "NEW_CMD");
    assert(slot != NULL);
    *slot = (void*)cmd_status;
    
    res = trie_get(&t, "NEW_CMD");
    assert(res == (void*)cmd_status);
    printf("[SUCCESS]\n");

    trie_clear(&t);

    printf("\n[GOOD] all passed\n");
    return 0;
}

#endif

