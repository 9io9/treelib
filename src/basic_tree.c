#include <stdlib.h>
#include <string.h>

#include "basic_tree.h"

static Result nfind(Node* root, CmpFn fcmp, void* data);
static Node* nmin(Node* n);
static Node* nmax(Node* n);

static Node* nmin(Node* n) {
    while (n->l != NULL) {
        n = n->l;
    }

    return n;
}

static Node* nmax(Node* n) {
    while (n->r != NULL) {
        n = n->r;
    }

    return n;
}

static order ndorder(Node* n) {
    return n->p->l == n ? left : right;
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

            default: return RESULT_FAIL("compare function returns invalid value", ECODE(TREELIB, BST, ICMP));
        }
    }

    return RESULT_FAIL("no data found in the tree", ECODE(TREELIB, BST, DNF));
}

Error basic_tree_init(BasicTree* bt, int dsize, CopyFn fcopy, CmpFn fcmp, SwapFn fswap) {
    if (bt == NULL || fcopy == NULL || fcmp == NULL || fswap == NULL) {
        return ERROR("bt == NULL or fcopy == NULL or fcmp == NULL or fswap == NULL", ECODE(TREELIB, BST, ARGV));
    }

    bt->fcopy = fcopy;
    bt->fcmp = fcmp;
    bt->fswap = fswap;

    Error e = fsalloc_init(&bt->allocator, dsize + sizeof(Node), 1, -1);

    if (e.message != NULL) {
        return e;
    }

    bt->ncnt = 0;
    bt->root = NULL;

    return ERROR(NULL, 0);
}

Error basic_tree_ainit(BasicTree* bt, int chunk_size, int pmin, int pmax, CopyFn fcopy, CmpFn fcmp, SwapFn fswap) {
    if (bt == NULL || fcopy == NULL || fcmp == NULL || fswap == NULL) {
        return ERROR("bt == NULL or fcopy == NULL or fcmp == NULL or fswap == NULL", ECODE(TREELIB, BST, ARGV));
    }

    bt->fcopy = fcopy;
    bt->fcmp = fcmp;
    bt->fswap = fswap;

    Error e = fsalloc_init(&bt->allocator, chunk_size + sizeof(Node), pmin, pmax);

    if (e.message != NULL) {
        return e;
    }

    bt->ncnt = 0;
    bt->root = NULL;

    return ERROR(NULL, 0);
}

Error basic_tree_put(BasicTree* bt, void* src) {
    if (bt == NULL || src == NULL) {
        return ERROR("bt == NULL or src == NULL", ECODE(TREELIB, BST, ARGV));
    }

    if (bt->ncnt == 0) {
        return ERROR("root node is NULL, tree is empty", ECODE(TREELIB, BST, EMPTY));
    }

    Result n = nfind(bt->root, bt->fcmp, src);

    if (n.has_value) {
        Node* nd = n.result.data.to_ptr;

        bt->fcopy(nd->d, src);

        return ERROR(NULL, 0);
    }

    return n.result.error;
}

Error basic_tree_get(BasicTree* bt, void* dest) {
    if (bt == NULL || dest == NULL) {
        return ERROR("bt == NULL or dest == NULL", ECODE(TREELIB, BST, ARGV));
    }

    if (bt->ncnt == 0) {
        return ERROR("root node is NULL, tree is empty", ECODE(TREELIB, BST, EMPTY));
    }

    Result n = nfind(bt->root, bt->fcmp, dest);

    if (n.has_value) {
        Node* nd = n.result.data.to_ptr;

        bt->fcopy(dest, nd->d);

        return ERROR(NULL, 0);
    }

    return n.result.error;
}

Error basic_tree_del(BasicTree* bt, void* data) {
    if (bt == NULL || data == NULL) {
        return ERROR("bt == NULL or data == NULL", ECODE(TREELIB, BST, ARGV));
    }

    if (bt->ncnt == 0) {
        return ERROR("root node is NULL, tree is empty", ECODE(TREELIB, BST, EMPTY));
    }

    if (bt->ncnt == 1) {
        bt->ncnt = 0;
        bt->fcopy(data, bt->root->d);
        return fsfree(&bt->allocator, (void**) &bt->root);
    }

    Result n = nfind(bt->root, bt->fcmp, data);

    if (n.has_value) {
        Node* nd = n.result.data.to_ptr;

        bt->fcopy(data, nd->d);
        bt->ncnt -= 1;

        if (nd->r != NULL) {
            Node* sd = nmin(nd->r);

            bt->fswap(nd->d, sd->d);
            
            if (ndorder(sd) == left) {
                sd->p->l = sd->r;
                sd->r->p = sd->p;
            } else {
                sd->p->r = sd->r;
                sd->r->p = sd->p;
            }

            return fsfree(&bt->allocator, (void**) &sd);
        }

        if (ndorder(nd) == left) {
            nd->p->l = nd->l;
            nd->l->p = nd->p;
        } else {
            nd->p->r = nd->l;
            nd->l->p = nd->p;
        }

        return fsfree(&bt->allocator, (void**) &nd);
    }

    return n.result.error;
}

Error basic_tree_min(BasicTree* bt, void* data) {
    if (bt == NULL || data == NULL) {
        return ERROR("bt == NULL or data == NULL", ECODE(TREELIB, BST, ARGV));
    }

    if (bt->ncnt == 0) {
        return ERROR("root node is NULL, tree is empty", ECODE(TREELIB, BST, EMPTY));
    }

    Result n = nfind(bt->root, bt->fcmp, data);

    if (n.has_value) {
        bt->fcopy(data, nmin(n.result.data.to_ptr)->d);
        return ERROR(NULL, 0);
    }

    return n.result.error;
}

Error basic_tree_max(BasicTree* bt, void* data) {
    if (bt == NULL || data == NULL) {
        return ERROR("bt == NULL or data == NULL", ECODE(TREELIB, BST, ARGV));
    }

    if (bt->ncnt == 0) {
        return ERROR("root node is NULL, tree is empty", ECODE(TREELIB, BST, EMPTY));
    }

    Result n = nfind(bt->root, bt->fcmp, data);

    if (n.has_value) {
        bt->fcopy(data, nmax(n.result.data.to_ptr)->d);
        return ERROR(NULL, 0);
    }

    return n.result.error;
}

Error basic_tree_lmax(BasicTree* bt, void* data) {
    if (bt == NULL || data == NULL) {
        return ERROR("bt == NULL or data == NULL", ECODE(TREELIB, BST, ARGV));
    }

    if (bt->ncnt == 0) {
        return ERROR("root node is NULL, tree is empty", ECODE(TREELIB, BST, EMPTY));
    }

    Result n = nfind(bt->root, bt->fcmp, data);

    if (n.has_value) {
        Node* nd = n.result.data.to_ptr;

        if (nd->l == NULL) {
            bt->fcopy(data, nd->d);
        } else {
            bt->fcopy(data, nmax(nd->l)->d);
        }

        return ERROR(NULL, 0);
    }

    return n.result.error;
}

Error basic_tree_rmin(BasicTree* bt, void* data) {
    if (bt == NULL || data == NULL) {
        return ERROR("bt == NULL or data == NULL", ECODE(TREELIB, BST, ARGV));
    }

    if (bt->ncnt == 0) {
        return ERROR("root node is NULL, tree is empty", ECODE(TREELIB, BST, EMPTY));
    }

    Result n = nfind(bt->root, bt->fcmp, data);

    if (n.has_value) {
        Node* nd = n.result.data.to_ptr;

        if (nd->r == NULL) {
            bt->fcopy(data, nd->d);
        } else {
            bt->fcopy(data, nmin(nd->r)->d);
        }

        return ERROR(NULL, 0);
    }

    return n.result.error;
}

Error basic_tree_new(BasicTree* bt, void* ndata) {
    if (bt == NULL || ndata == NULL) {
        return ERROR("bt == NULL or ndata == NULL", ECODE(TREELIB, BST, ARGV));
    }

    if (bt->ncnt == 0) {
        Result nr = fsalloc(&bt->allocator);

        if (nr.has_value) {
            bt->root = nr.result.data.to_ptr;
            memset(bt->root, 0, sizeof(Node));
            bt->fcopy(bt->root->d, ndata);
            bt->ncnt += 1;
            return ERROR(NULL, 0);
        }

        return nr.result.error;
    }

    Node* ndp = bt->root;

    while (true) {
        switch (bt->fcmp(ndp->d, ndata)) {
            case left: {
                if (ndp->l == NULL) {
                    Result nr = fsalloc(&bt->allocator);

                    if (nr.has_value) {
                        ndp->l = nr.result.data.to_ptr;
                        memset(ndp->l, 0, sizeof(Node));
                        ndp->l->p = ndp;
                        bt->fcopy(ndp->l->d, ndata);
                        bt->ncnt += 1;
                        return ERROR(NULL, 0);
                    }

                    return nr.result.error;
                }

                ndp = ndp->l;
                break;
            }

            case right: {
                if (ndp->r == NULL) {
                    Result nr = fsalloc(&bt->allocator);

                    if (nr.has_value) {
                        ndp->r = nr.result.data.to_ptr;
                        memset(ndp->r, 0, sizeof(Node));
                        ndp->r->p = ndp;
                        bt->fcopy(ndp->r->d, ndata);
                        bt->ncnt += 1;
                        return ERROR(NULL, 0);
                    }

                    return nr.result.error;
                }

                ndp = ndp->r;
                break;
            }

            case equal: {
                return ERROR("node already inserted", ECODE(TREELIB, BST, DUP));
            }

            default: return ERROR("invalid return value of compare function", ECODE(TREELIB, BST, ICMP));
        }
    }
}

Error basic_tree_trav(BasicTree* bt, torder o, VisFn fvisit);

Error basic_tree_destroy(BasicTree* bt) {
    if (bt == NULL) {
        return ERROR("bt == NULL", ECODE(TREELIB, BST, ARGV));
    }

    Error error = fsalloc_destroy(&bt->allocator);

    if (error.message != NULL) {
        return error;
    }

    memset(bt, 0, sizeof(BasicTree));
}