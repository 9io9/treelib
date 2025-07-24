#ifndef _LJW_TREELIB_BASIC_TREE_H_

#define _LJW_TREELIB_BASIC_TREE_H_

#include "utils/result.h"
#include "fsalloc.h"

typedef enum {
    left, right, equal
}order;

typedef enum {
    pre, in, post
}torder; // trav order

typedef void (*CopyFn) (void* /* dest */, void* /* src */); // copy content from arg 2 to arg 1
typedef order (*CmpFn) (void* /* data in node */, void* /* data out of node */); // if data out of node less than data in node, return left, otherwise return right 
typedef void (*VisFn) (void* /* data in node */);

typedef struct {
    void* l; // left
    void* r; // right
    void* p; // parent
    char d[]; // data
}Node;

typedef struct {
    int ncnt; // nodes count
    Node* root;
    CopyFn fcopy;
    CmpFn fcmp;
    FsAllocator allocator;
}BasicTree;

Error basic_tree_init(BasicTree* bt, int dsize, CopyFn fcopy, CmpFn fcmp);
Error basic_tree_ainit(BasicTree* bt, int chunk_size, int pmin, int pmax, CopyFn fcopy, CmpFn fcmp);
Error basic_tree_put(BasicTree* bt, void* src);
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