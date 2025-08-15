#ifndef _LJW_TREELIB_BASIC_TREE_H_

#define _LJW_TREELIB_BASIC_TREE_H_

#include "chdrs/result.h"
#include "chdrs/fn.h"
#include "aLocas/include/fsalloc.h"
#include "ec.h"

typedef enum {
    pre, in, post
}torder; // trav order

typedef struct Node {
    struct Node* l; // left
    struct Node* r; // right
    struct Node* p; // parent
    char d[]; // data
}Node;

typedef struct {
    int ncnt; // nodes count
    Node* root;
    CopyFn fcopy;
    CmpFn fcmp;
    SwapFn fswap;
    FsAllocator allocator;
}BasicTree;

Error basic_tree_init(BasicTree* bt, int dsize, CopyFn fcopy, CmpFn fcmp, SwapFn fswap);
Error basic_tree_ainit(BasicTree* bt, int chunk_size, int pmin, int pmax, CopyFn fcopy, CmpFn fcmp, SwapFn fswap);
Error basic_tree_put(BasicTree* bt, void* src);
Error basic_tree_repl(BasicTree* bt, void* dest, void* src);
Error basic_tree_get(BasicTree* bt, void* dest);
Error basic_tree_min(BasicTree* bt, void* data);
Error basic_tree_max(BasicTree* bt, void* data);
Error basic_tree_lmax(BasicTree* bt, void* data);
Error basic_tree_rmin(BasicTree* bt, void* data);
Error basic_tree_del(BasicTree* bt, void* data);
Error basic_tree_new(BasicTree* bt, void* ndata);
Error basic_tree_trav(BasicTree* bt, torder o, VisFn fvisit);
Error basic_tree_destroy(BasicTree* bt);

#endif