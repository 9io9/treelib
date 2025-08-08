#include <stddef.h>
#include <string.h>
#include "btree.h"

typedef struct {
    BNode* l;
    BNode* r;
    void* d;
}BPair;

static inline void* at(void* base, int offset, int size) {
    return base + offset * size;
}

static inline int ndsize(int dsize, int ncap) {
    return sizeof(BNode) + dsize * ncap + sizeof(BNode*) * (ncap + 1);
}

static Error bpat(BNode* bnd, BPair* bp, int i, int dsize, int ncap) {
    if (i >= bnd->nsize) {
        return ERROR("out of bound for bnode", ECODE(TREELIB, BT, OBD));
    }

    bp->d = at(bnd->ds, i, dsize);
    bp->l = at(at(bnd->ds, ncap, dsize), i, sizeof(BNode*));
    bp->r = at(at(bnd->ds, ncap, dsize), i + 1, sizeof(BNode*));

    return ERROR(NULL, 0);
}

static void bshift(BNode* bnd, int begin, int dsize, CopyFn fcopy) {
    for (int i = bnd->nsize - 1; i >= begin; --i) {
        fcopy(at(bnd->ds, i + 1, dsize), at(bnd->ds, i, dsize));
    }
}

static Result nfind(BNode* root, void* data, int* index) {
    
}