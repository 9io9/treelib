#include <stddef.h>
#include <string.h>
#include "rb_tree.h"

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

static void lr(BasicTree* rbt, Node* n) {
    Node* nr = n->r;
    
    n->r = nr->l;

    if (nr->l != NULL) {
        nr->l->p = n;
    }

    nr->p = n->p;

    if (n->p == NULL) {
        rbt->root = nr;
    } else if (n == n->p->l) {
        n->p->l = nr;
    } else {
        n->p->r = nr;
    }

    nr->l = n;
    n->p = nr;
}

static void rr(BasicTree* rbt, Node* n) {
    Node* nl = n->l;

    n->l = nl->r;

    if (nl->r != NULL) {
        nl->r->p = n;
    }
        
    nl->p = n->p;

    if (n->p == NULL) {
        rbt->root = nl;
    } else if (n == n->p->r) {
        n->p->r = nl;
    } else {
        n->p->l = nl;
    }
        
    nl->r = n;
    n->p = nl;
}

static ndcolor ncolor(Node* n) {
    return n == NULL ? black : ((RBNode*) n->d)->color;
}

static void sncolor(Node* n, ndcolor c) {
    if (n == NULL) {
        return;
    }

    ((RBNode*) n->d)->color = c;
}

static inline void* ndat(Node* n) {
    return ((RBNode*) n->d)->d;
}

static Result nfind(Node* root, CmpFn fcmp, void* data) {
    while (root != NULL) {
        switch (fcmp(ndat(root), data)) {
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

            default: return RESULT_FAIL("compare function returns invalid value", ECODE(TREELIB, RBT, ICMP));
        }
    }

    return RESULT_FAIL("no data found in the tree", ECODE(TREELIB, RBT, DNF));
}

static void pre_trav(Node* root, VisFn fvisit) {
    if (root == NULL) {
        return;
    }

    Node* nd = root;

    while (true) {
        fvisit(ndat(nd));

        if (nd->l == NULL) {
            pre_trav(nd->r, fvisit);
            
            while (nd != root) {
                nd = nd->p;
                pre_trav(nd->r, fvisit);
            }

            return;
        }

        nd = nd->l;
    }
}

static void in_trav(Node* root, VisFn fvisit) {
    if (root == NULL) {
        return;
    }

    Node* nd = root;

    while (true) {
        if (nd->l == NULL) {
            fvisit(ndat(nd));
            in_trav(nd->r, fvisit);
            
            while (nd != root) {
                nd = nd->p;
                fvisit(ndat(nd));
                in_trav(nd->r, fvisit);
            }

            return;
        }

        nd = nd->l;
    }
}

static void post_trav(Node* root, VisFn fvisit) {
    if (root == NULL) {
        return;
    }

    Node* nd = root;

    while (true) {
        if (nd->l == NULL) {
            post_trav(nd->r, fvisit);
            fvisit(ndat(nd));

            while (nd != root) {
                nd = nd->p;
                post_trav(nd->r, fvisit);
                fvisit(ndat(nd));
            }

            return;
        }

        nd = nd->l;
    }
}

static void ibalance(BasicTree* rbt, Node* n) {
    while (ncolor(n->p) == red) {
        if (n->p == n->p->p->l) {
            Node* nppr = n->p->p->r;

            if (ncolor(nppr) == red) {
                sncolor(n->p, black);
                sncolor(nppr, black);
                sncolor(n->p->p, red);
                n = n->p->p;
            } else {
                if (n == n->p->r) {
                    n = n->p;
                    lr(rbt, n);
                }

                sncolor(n->p, black);
                sncolor(n->p->p, red);
                rr(rbt, n->p->p);
            }
        } else {
            Node* nppl = n->p->p->l;

            if (ncolor(nppl) == red) {
                sncolor(n->p, black);
                sncolor(nppl, black);
                sncolor(n->p->p, red);
                n = n->p->p;
            } else {
                if (n == n->p->l) {
                    n = n->p;
                    rr(rbt, n);
                }

                sncolor(n->p, black);
                sncolor(n->p->p, red);
                lr(rbt, n->p->p);
            }
        }
    }

    sncolor(rbt->root, black);
}

static void transplant(BasicTree* rbt, Node* n1, Node* n2) {
    if (n1->p == NULL) {
        rbt->root = n2;
    } else if (n1 == n1->p->l) {
        n1->p->l = n2;
    } else {
        n1->p->r = n2;
    }

    if (n2 != NULL) {
        n2->p = n1->p;
    }
}

static void dbalance(BasicTree* rbt, Node* n) {
    while (n != rbt->root && ncolor(n) == black) {
        if (n == n->p->l) {
            Node* npr = n->p->r;
            
            if (ncolor(npr) == red) {
                sncolor(npr, black);
                sncolor(n->p, red);
                lr(rbt, n->p);
                npr = n->p->r;
            }

            if (ncolor(npr->l) == black && ncolor(npr->r) == black) {
                sncolor(npr, red);
                n = n->p;
            } else {
                if (ncolor(npr->r) == black) {
                    sncolor(npr->l, black);
                    sncolor(npr, red);
                    rr(rbt, npr);
                    npr = n->p->r;
                }

                sncolor(npr, ncolor(n->p));
                sncolor(n->p, black);
                sncolor(npr->r, black);
                lr(rbt, n->p);
                n = rbt->root;
            }
        } else {
            Node* npl = n->p->l;

            if (ncolor(npl) == red) {
                sncolor(npl, black);
                sncolor(n->p, red);
                rr(rbt, n->p);
                npl = n->p->l;
            }
            if (ncolor(npl->r) == black && ncolor(npl->l) == black) {
                sncolor(npl, red);
                n = n->p;
            } else {
                if (ncolor(npl->l) == black) {
                    sncolor(npl->r, black);
                    sncolor(npl, red);
                    lr(rbt, npl);
                    npl = n->p->l;
                }
                
                sncolor(npl, ncolor(n->p));
                sncolor(n->p, black);
                sncolor(npl->l, black);
                rr(rbt, n->p);
                n = rbt->root;
            }
        }
    }

    sncolor(n, black);
}

Error rb_tree_init(BasicTree* rbt, int dsize, CopyFn fcopy, CmpFn fcmp, SwapFn fswap) {
    return basic_tree_init(rbt, dsize + sizeof(RBNode), fcopy, fcmp, fswap);
}

Error rb_tree_ainit(BasicTree* rbt, int chunk_size, int pmin, int pmax, CopyFn fcopy, CmpFn fcmp, SwapFn fswap) {
    return basic_tree_ainit(rbt, chunk_size + sizeof(RBNode), pmin, pmax, fcopy, fcmp, fswap);
}

Error rb_tree_new(BasicTree* rbt, void* ndata) {
    if (rbt == NULL || ndata == NULL) {
        return ERROR("rbt == NULL or ndata == NULL", ECODE(TREELIB, RBT, ARGV));
    }

    if (rbt->ncnt == 0) {
        Result nr = fsalloc(&rbt->allocator);

        if (nr.has_value) {
            rbt->root = nr.result.data.to_ptr;
            memset(rbt->root, 0, sizeof(Node));
            sncolor(rbt->root, black);
            rbt->fcopy(ndat(rbt->root), ndata);
            rbt->ncnt += 1;
            return ERROR(NULL, 0);
        }

        return nr.result.error;
    }

    Node* ndp = rbt->root;

    while (true) {
        switch (rbt->fcmp(ndat(ndp), ndata)) {
            case left: {
                if (ndp->l == NULL) {
                    Result nr = fsalloc(&rbt->allocator);

                    if (nr.has_value) {
                        ndp->l = nr.result.data.to_ptr;
                        memset(ndp->l, 0, sizeof(Node));
                        ndp->l->p = ndp;
                        sncolor(ndp->l, red);
                        rbt->fcopy(ndat(ndp->l), ndata);
                        rbt->ncnt += 1;
                        ibalance(rbt, ndp->l);
                        return ERROR(NULL, 0);
                    }

                    return nr.result.error;
                }

                ndp = ndp->l;
                break;
            }

            case right: {
                if (ndp->r == NULL) {
                    Result nr = fsalloc(&rbt->allocator);

                    if (nr.has_value) {
                        ndp->r = nr.result.data.to_ptr;
                        memset(ndp->r, 0, sizeof(Node));
                        ndp->r->p = ndp;
                        sncolor(ndp->r, red);
                        rbt->fcopy(ndat(ndp->r), ndata);
                        rbt->ncnt += 1;
                        ibalance(rbt, ndp->r);
                        return ERROR(NULL, 0);
                    }

                    return nr.result.error;
                }

                ndp = ndp->r;
                break;
            }

            case equal: {
                return ERROR("node already inserted", ECODE(TREELIB, RBT, DUP));
            }

            default: return ERROR("invalid return value of compare function", ECODE(TREELIB, RBT, ICMP));
        }
    }
}

Error rb_tree_del(BasicTree* rbt, void* data) {
    if (rbt == NULL || data == NULL) {
        return ERROR("rbt == NULL or data == NULL", ECODE(TREELIB, RBT, ARGV));
    }

    if (rbt->ncnt == 0) {
        return ERROR("tree root is NULL, tree is empty", ECODE(TREELIB, RBT, EMPTY));
    }

    Result n = nfind(rbt->root, rbt->fcmp, data);

    if (n.has_value) {
        Node* nd = n.result.data.to_ptr;
        Node* ny = nd;
        Node* nx;
        ndcolor nyc = ncolor(ny);

        if (nd->l == NULL) {
            nx = nd->r;
            transplant(rbt, nd, nd->r);
        } else if (nd->r == NULL) {
            nx = nd->l;
            transplant(rbt, nd, nd->l);
        } else {
            ny = nmin(nd->r);
            nyc = ncolor(ny);
            nx = ny->r;

            if (ny->p == nd) {
                if (nx != NULL) {
                    nx->p = ny;
                }
            } else {
                transplant(rbt, ny, ny->r);
                ny->r = nd->r;

                if (ny->r != NULL) {
                    ny->r->p = ny;
                }
            }

            transplant(rbt, nd, ny);
            ny->l = nd->l;
            
            if (ny->l != NULL) {
                ny->l->p = ny;
            }

            sncolor(ny, ncolor(nd));
        }

        if (nyc == black) {
            dbalance(rbt, nx);
        }

        return fsfree(&rbt->allocator, (void**) &nd);
    }

    return n.result.error;
}

Error rb_tree_put(BasicTree* rbt, void* src) {
    if (rbt == NULL || src == NULL) {
        return ERROR("bt == NULL or src == NULL", ECODE(TREELIB, RBT, ARGV));
    }

    if (rbt->ncnt == 0) {
        return ERROR("root node is NULL, tree is empty", ECODE(TREELIB, RBT, EMPTY));
    }

    Result n = nfind(rbt->root, rbt->fcmp, src);

    if (n.has_value) {
        Node* nd = n.result.data.to_ptr;

        rbt->fcopy(ndat(nd), src);

        return ERROR(NULL, 0);
    }

    return n.result.error;
}

Error rb_tree_get(BasicTree* rbt, void* dest) {
    if (rbt == NULL || dest == NULL) {
        return ERROR("bt == NULL or dest == NULL", ECODE(TREELIB, RBT, ARGV));
    }

    if (rbt->ncnt == 0) {
        return ERROR("root node is NULL, tree is empty", ECODE(TREELIB, RBT, EMPTY));
    }

    Result n = nfind(rbt->root, rbt->fcmp, dest);

    if (n.has_value) {
        Node* nd = n.result.data.to_ptr;

        rbt->fcopy(dest, ndat(nd));

        return ERROR(NULL, 0);
    }

    return n.result.error;
}

Error rb_tree_min(BasicTree* rbt, void* data) {
    if (rbt == NULL || data == NULL) {
        return ERROR("bt == NULL or data == NULL", ECODE(TREELIB, RBT, ARGV));
    }

    if (rbt->ncnt == 0) {
        return ERROR("root node is NULL, tree is empty", ECODE(TREELIB, RBT, EMPTY));
    }

    Result n = nfind(rbt->root, rbt->fcmp, data);

    if (n.has_value) {
        rbt->fcopy(data, ndat(nmin(n.result.data.to_ptr)));
        return ERROR(NULL, 0);
    }

    return n.result.error;
}

Error rb_tree_max(BasicTree* rbt, void* data) {
    if (rbt == NULL || data == NULL) {
        return ERROR("bt == NULL or data == NULL", ECODE(TREELIB, RBT, ARGV));
    }

    if (rbt->ncnt == 0) {
        return ERROR("root node is NULL, tree is empty", ECODE(TREELIB, RBT, EMPTY));
    }

    Result n = nfind(rbt->root, rbt->fcmp, data);

    if (n.has_value) {
        rbt->fcopy(data, ndat(nmax(n.result.data.to_ptr)));
        return ERROR(NULL, 0);
    }

    return n.result.error;
}

Error rb_tree_lmax(BasicTree* rbt, void* data) {
    if (rbt == NULL || data == NULL) {
        return ERROR("bt == NULL or data == NULL", ECODE(TREELIB, RBT, ARGV));
    }

    if (rbt->ncnt == 0) {
        return ERROR("root node is NULL, tree is empty", ECODE(TREELIB, RBT, EMPTY));
    }

    Result n = nfind(rbt->root, rbt->fcmp, data);

    if (n.has_value) {
        Node* nd = n.result.data.to_ptr;

        if (nd->l == NULL) {
            rbt->fcopy(data, ndat(nd));
        } else {
            rbt->fcopy(data, ndat(nmax(nd->l)));
        }

        return ERROR(NULL, 0);
    }

    return n.result.error;
}

Error rb_tree_rmin(BasicTree* rbt, void* data) {
    if (rbt == NULL || data == NULL) {
        return ERROR("bt == NULL or data == NULL", ECODE(TREELIB, RBT, ARGV));
    }

    if (rbt->ncnt == 0) {
        return ERROR("root node is NULL, tree is empty", ECODE(TREELIB, RBT, EMPTY));
    }

    Result n = nfind(rbt->root, rbt->fcmp, data);

    if (n.has_value) {
        Node* nd = n.result.data.to_ptr;

        if (nd->r == NULL) {
            rbt->fcopy(data, ndat(nd));
        } else {
            rbt->fcopy(data, ndat(nmin(nd->r)));
        }

        return ERROR(NULL, 0);
    }

    return n.result.error;
}

Error rb_tree_trav(BasicTree* rbt, torder o, VisFn fvisit) {
    if (rbt == NULL || fvisit == NULL) {
        return ERROR("bt == NULL or fvisit == NULL", ECODE(TREELIB, RBT, ARGV));
    }

    switch (o) {
        case pre: {
            pre_trav(rbt->root, fvisit);
            break;
        }

        case in: {
            in_trav(rbt->root, fvisit);
            break;
        }

        case post: {
            post_trav(rbt->root, fvisit);
            break;
        }

        default: return ERROR("invalid traverse order", ECODE(TREELIB, RBT, ITOD));
    }

    return ERROR(NULL, 0);
}

Error rb_tree_destroy(BasicTree* rbt) {
    if (rbt == NULL) {
        return ERROR("bt == NULL", ECODE(TREELIB, RBT, ARGV));
    }

    Error error = fsalloc_destroy(&rbt->allocator);

    if (error.message != NULL) {
        return error;
    }

    memset(rbt, 0, sizeof(BasicTree));
}