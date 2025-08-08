#ifndef _LJW_TREELIB_BTREE_H_

#define _LJW_TREELIB_BTREE_H_

#include "chdrs/fn.h"
#include "aLocas/include/fsalloc.h"
#include "ec.h"

typedef struct BNode {
    int nsize;
    char ds[];
}BNode;

typedef struct {
    int ncnt;
    int ncap;
    CopyFn fcopy;
    CmpFn fcmp;
    SwapFn fswap;
    FsAllocator allocator;
}BTree;

Error btree_init(BTree* bt, int dsize, int ncap, CopyFn fcopy, CmpFn fcmp, SwapFn fswap);
Error btree_ainit(BTree* bt, int chunk_size, int pmin, int pmax, int ncap, CopyFn fcopy, CmpFn fcmp, SwapFn fswap);
Error btree_get(BTree* bt, void* dest);
Error btree_put(BTree* bt, void* src);
Error btree_new(BTree* bt, void* ndata);
Error btree_del(BTree* bt, void* data);
Error btree_destroy(BTree* bt);

#endif