#ifndef _LJW_TREELIB_TRIE_H_

#define _LJW_TREELIB_TRIE_H_

#include "aLocas/include/fsalloc.h"
#include "chdrs/fn.h"
#include "ec.h"

typedef struct TrieNode {
    bool is_term;
    char c;
    struct TrieNode* ls;
    struct TrieNode* eq;
    struct TrieNode* gt;
    char d[];
}TrieNode;

typedef struct {
    TrieNode* root;
    CopyFn fcopy;
    FsAllocator allocator;
}Trie;

Error trie_init(Trie* trie, int dsize, CopyFn fcopy);
Error trie_ainit(Trie* trie, int chunk_size, int pmin, int pmax, CopyFn fcopy);
Error trie_put(Trie* trie, const char* s, void* v);
Error trie_get(Trie* trie, const char* s, void* v);
Error trie_del(Trie* trie, const char* s, void* v);
Error trie_destroy(Trie* trie);

#endif