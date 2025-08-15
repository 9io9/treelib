#ifndef _LJW_TREELIB_RB_TREE_H_

#define _LJW_TREELIB_RB_TREE_H_

#include "basic_tree.h"

typedef enum {
    red, black
}ndcolor;

typedef struct {
    ndcolor color;
    char d[];
}RBNode;

Error rb_tree_init(BasicTree* rbt, int dsize, CopyFn fcopy, CmpFn fcmp, SwapFn fswap);
Error rb_tree_ainit(BasicTree* rbt, int chunk_size, int pmin, int pmax, CopyFn fcopy, CmpFn fcmp, SwapFn fswap);
Error rb_tree_new(BasicTree* rbt, void* ndata);
Error rb_tree_del(BasicTree* rbt, void* data);
Error rb_tree_put(BasicTree* rbt, void* src);
Error rb_tree_repl(BasicTree* rbt, void* dest, void* src);
Error rb_tree_get(BasicTree* rbt, void* dest);
Error rb_tree_min(BasicTree* rbt, void* data);
Error rb_tree_max(BasicTree* rbt, void* data);
Error rb_tree_lmax(BasicTree* rbt, void* data);
Error rb_tree_rmin(BasicTree* rbt, void* data);
Error rb_tree_trav(BasicTree* rbt, torder o, VisFn fvisit);
Error rb_tree_destroy(BasicTree* rbt);

#endif