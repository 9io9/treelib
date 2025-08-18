#include <string.h>
#include "trie.h"

Error trie_init(Trie* trie, int dsize, CopyFn fcopy) {
    if (trie == NULL || fcopy == NULL) {
        return ERROR("trie == NULL or fcopy == NULL", ECODE(TREELIB, TRIE, ARGV));
    }

    trie->fcopy = fcopy;
    trie->root = NULL;
    
    return fsalloc_init(&trie->allocator, sizeof(TrieNode) + dsize, 1, -1);
}

Error trie_ainit(Trie* trie, int chunk_size, int pmin, int pmax, CopyFn fcopy) {
    if (trie == NULL || fcopy == NULL) {
        return ERROR("trie == NULL or fcopy == NULL", ECODE(TREELIB, TRIE, ARGV));
    }

    trie->fcopy = fcopy;
    trie->root = NULL;
    
    return fsalloc_init(&trie->allocator, sizeof(TrieNode) + chunk_size, pmin, pmax);
}

static Error svput(const char* s, TrieNode* n, void* v, CopyFn fcopy, FsAllocator* allocator) {
    Result r;

    while (true) {
        n->c = s[0];
        ++s;

        if (s[0] == '\0') {
            n->is_term = true;
            fcopy(n->d, v);
            return ERROR(NULL, 0);
        }

        r = fsalloc(allocator);

        if (!r.has_value) {
            return r.result.error;
        }

        n->eq = r.result.data.to_ptr;
        n = n->eq;
        n->ls = NULL;
        n->gt == NULL;
    }
}

Error trie_put(Trie* trie, const char* s, void* v) {
    if (trie == NULL || s == NULL || v == NULL || s[0] == '\0') {
        return ERROR("trie == NULL or s == NULL or v == NULL or s with zero length", ECODE(TREELIB, TRIE, ARGV));
    }

    TrieNode* n = trie->root;
    Result r;

    if (n == NULL) {
        r = fsalloc(&trie->allocator);

        if (!r.has_value) {
            return r.result.error;
        }

        n = r.result.data.to_ptr;

        n->ls = n->gt = NULL;

        return svput(s, n, v, trie->fcopy, &trie->allocator);
    }

    while (s[0] != '\0') {
        if (n->c == s[0]) {
            if (n->eq == NULL) {
                r = fsalloc(&trie->allocator);

                if (!r.has_value) {
                    return r.result.error;
                }

                n->eq = r.result.data.to_ptr;

                return svput(s + 1, n->eq, v, trie->fcopy, &trie->allocator);
            }

            n = n->eq;
        } else if (n->c > s[0]) {
            if (n->ls == NULL) {
                r = fsalloc(&trie->allocator);

                if (!r.has_value) {
                    return r.result.error;
                }

                n->ls = r.result.data.to_ptr;

                return svput(s + 1, n->ls, v, trie->fcopy, &trie->allocator);
            }

            n = n->ls;
        } else {
            if (n->gt == NULL) {
                r = fsalloc(&trie->allocator);

                if (!r.has_value) {
                    return r.result.error;
                }

                n->gt = r.result.data.to_ptr;

                return svput(s + 1, n->gt, v, trie->fcopy, &trie->allocator);
            }

            n = n->gt;
        }

        ++s;
    }

    return ERROR(NULL, 0);
}

Error trie_get(Trie* trie, const char* s, void* v) {
    if (trie == NULL || s == NULL || v == NULL || s[0] == '\0') {
        return ERROR("trie == NULL or s == NULl or v == NULL or s with zero length", ECODE(TREELIB, TRIE, ARGV));
    }

    TrieNode* n = trie->root;

    if (n == NULL) {
        return ERROR("trie is empty", ECODE(TREELIB, TRIE, EMPTY));
    }

    while (s[0] != '\0' && n != NULL) {
        if (n->c == s[0]) {
            n = n->eq;
        } else if (n->c > s[0]) {
            n = n->ls;
        } else {
            n = n->gt;
        }

        ++s;
    }

    if (n == NULL) {
        return ERROR("data not found for trie", ECODE(TREELIB, TRIE, DNF));
    }

    if (!n->is_term) {
        return ERROR("data not found for trie", ECODE(TREELIB, TRIE, DNF));
    }

    trie->fcopy(v, n->d);
    
    return ERROR(NULL, 0);
}

Error trie_del(Trie* trie, const char* s, void* v) {
    if (trie == NULL || s == NULL || v == NULL || s[0] == '\0') {
        return ERROR("trie == NULL or s == NULl or v == NULL or s with zero length", ECODE(TREELIB, TRIE, ARGV));
    }

    TrieNode* n = trie->root;

    if (n == NULL) {
        return ERROR("trie is empty", ECODE(TREELIB, TRIE, EMPTY));
    }

    while (s[0] != '\0' && n != NULL) {
        if (n->c == s[0]) {
            n = n->eq;
        } else if (n->c > s[0]) {
            n = n->ls;
        } else {
            n = n->gt;
        }

        ++s;
    }

    if (n == NULL) {
        return ERROR("data not found for trie", ECODE(TREELIB, TRIE, DNF));
    }

    if (!n->is_term) {
        return ERROR("data not found for trie", ECODE(TREELIB, TRIE, DNF));
    }

    n->is_term = false;
    trie->fcopy(v, n->d);

    return ERROR(NULL, 0);
}

Error trie_destroy(Trie* trie) {
    if (trie == NULL) {
        return ERROR("trie == NULL", ECODE(TREELIB, TRIE, ARGV));
    }

    Error error = fsalloc_destroy(&trie->allocator);

    if (error.message != NULL) {
        return error;
    }

    memset(trie, 0, sizeof(Trie));

    return error;
}