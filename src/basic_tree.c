#include <stdlib.h>

#include "basic_tree.h"

static Result nfind(Node* root, CmpFn fcmp, void* data);
static Node* min(Node* n);
static Node* max(Node* n);

static Node* min(Node* n) {
    while (n->l != NULL) {
        n = n->l;
    }

    return n;
}

static Node* max(Node* n) {
    while (n->r != NULL) {
        n = n->r;
    }

    return n;
}

static Result nfind(Node* root, CmpFn fcmp, void* data) {
    while (root != NULL) {
        switch (fcmp(root->d, data)) {
            case left: {
                root = root->l;
                break;
            }

            case right: {
                root = root->r;
                break;
            }

            case equal: {
                return RESULT_SUC(ptr, root);
            }

            default: return RESULT_FAIL("compare function returns invalid value");
        }
    }

    return RESULT_FAIL("no data found in the tree");
}

Error basic_tree_init(BasicTree* bt, int dsize, CopyFn fcopy, CmpFn fcmp) {
    if (bt == NULL || fcopy == NULL || fcmp == NULL) {
        return ERROR("bt == NULL or fcopy == NULL or fcmp == NULL");
    }

    bt->fcopy = fcopy;
    bt->fcmp = fcmp;

    Error e = fsalloc_init(&bt->allocator, dsize + sizeof(Node), 1, -1);

    if (e.message != NULL) {
        return e;
    }

    bt->ncnt = 0;
    bt->root = NULL;

    return ERROR(NULL);
}

Error basic_tree_ainit(BasicTree* bt, int chunk_size, int pmin, int pmax, CopyFn fcopy, CmpFn fcmp) {
    if (bt == NULL || fcopy == NULL || fcmp == NULL) {
        return ERROR("bt == NULL or fcopy == NULL or fcmp == NULL");
    }

    bt->fcopy = fcopy;
    bt->fcmp = fcmp;

    Error e = fsalloc_init(&bt->allocator, chunk_size + sizeof(Node), pmin, pmax);

    if (e.message != NULL) {
        return e;
    }

    bt->ncnt = 0;
    bt->root = NULL;

    return ERROR(NULL);
}

Error basic_tree_put(BasicTree* bt, void* src) {
    if (bt == NULL || src == NULL) {
        return ERROR("bt == NULL or src == NULL");
    }

    if (bt->root == NULL) {
        return ERROR("root node is NULL, tree is empty");
    }

    Result n = nfind(bt->root, bt->fcmp, src);

    if (n.has_value) {
        Node* nd = n.result.data.to_ptr;

        bt->fcopy(nd->d, src);

        return ERROR(NULL);
    }

    return ERROR(n.result.error.message);
}

Error basic_tree_get(BasicTree* bt, void* dest) {
    if (bt == NULL || dest == NULL) {
        return ERROR("bt == NULL or dest == NULL");
    }

    if (bt->root == NULL) {
        return ERROR("root node is NULL, tree is empty");
    }

    Result n = nfind(bt->root, bt->fcmp, dest);

    if (n.has_value) {
        Node* nd = n.result.data.to_ptr;

        bt->fcopy(dest, nd->d);

        return ERROR(NULL);
    }

    return ERROR(n.result.error.message);
}

Error basic_tree_del(BasicTree* bt, void* data) {
    if (bt == NULL || data == NULL) {
        return ERROR("bt == NULL or data == NULL");
    }

    if (bt->root == NULL) {
        return ERROR("root node is NULL, tree is empty");
    }

    Result n = nfind(bt->root, bt->fcmp, data);

    if (n.has_value) {
        
    }
}

Error basic_tree_min(BasicTree* bt, void* data);
Error basic_tree_max(BasicTree* bt, void* data);
Error basic_tree_lmax(BasicTree* bt, void* data);
Error basic_tree_rmin(BasicTree* bt, void* data);
Error basic_tree_new(BasicTree* bt, void* ndata);
Error basic_tree_trav(BasicTree* bt, torder o, VisFn fvisit);
Error basic_tree_destroy(BasicTree* bt);