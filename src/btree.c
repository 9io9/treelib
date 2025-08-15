#include <stddef.h>
#include <string.h>
#include "seqlib/include/algo/search.h"
#include "seqlib/include/vec.h"
#include "seqlib/include/ec.h"
#include "btree.h"

typedef struct {
    BNode* root;
    BNode* l;
    BNode* r;
}BGroup;

static inline void* at(void* base, int offset, int size) {
    return base + offset * size;
}

static inline int ndsize(int dsize, int ncap) {
    return sizeof(BNode) + dsize * ncap + sizeof(BNode*) * (ncap + 1);
}

static inline void* bdat(BNode* bnd, int i, int dsize) {
    return at(bnd->ds, i, dsize);
}

static inline BNode** bdpat(BNode* bnd, int i, int dsize, int ncap) {
    return at(at(bnd->ds, ncap, dsize), i, sizeof(BNode*));
}

static void bshift(BNode* bnd, int begin, int offset, int dsize, int ncap, CopyFn fcopy) {
    if (begin >= bnd->nsize) {
        return;
    }

    for (int i = bnd->nsize - 1; i >= begin; --i)
        fcopy(bdat(bnd, i + offset, dsize), bdat(bnd, i, dsize));
    for (int i = bnd->nsize; i >= begin; --i) {
        if (*bdpat(bnd, i, dsize, ncap) != NULL) {
            (*bdpat(bnd, i, dsize, ncap))->pindex += offset;
        }

        *bdpat(bnd, i + offset, dsize, ncap) = *bdpat(bnd, i, dsize, ncap);
    }
}

static Error n2grp(BNode* bnd, BGroup* grp, range grng[3], int ncap, CopyFn fcopy, FsAllocator* allocator) {
    int dsize = grng[0].s;
    int ll = (grng[0].end - grng[0].begin) / dsize + 1;
    int ml = (grng[1].end - grng[1].begin) / dsize + 1;
    int rl = (grng[2].end - grng[2].begin) / dsize + 1;
    Result r;

    grp->l = bnd;

    if (allocator != NULL) {
        r = fsalloc(allocator);

        if (!r.has_value) {
            return r.result.error;
        }

        grp->root = r.result.data.to_ptr;

        memset(grp->root, 0, ndsize(dsize, ncap));

        r = fsalloc(allocator);

        if (!r.has_value) {
            return r.result.error;
        }

        grp->r = r.result.data.to_ptr;

        memset(grp->r, 0, ndsize(dsize, ncap));
    }

    if (bnd->p != NULL) {
        *bdpat(bnd->p, bnd->pindex, dsize, ncap) = grp->root;
    }

    grp->root->p = bnd->p;
    grp->root->pindex = bnd->pindex;
    grp->l->p = grp->root;
    grp->l->pindex = 0;
    grp->r->p = grp->root;
    grp->r->pindex = ml;

    // copy data to root node and ptrs to root node, and modify all childrens' p and pindex to root node's
    for (int i = 0; i < ml; ++i)
        fcopy(bdat(grp->root, i, dsize), bdat(bnd, ll + i, dsize));
    
    *bdpat(grp->root, 0, dsize, ncap) = bnd;
    memcpy(bdpat(grp->root, 1, dsize, ncap), bdpat(bnd, ll + 1, dsize, ncap), sizeof(BNode*) * (ml - 1));
    *bdpat(grp->root, ml, dsize, ncap) = grp->r;

    for (int i = 1; i < ml; ++i) {
        BNode* child = *bdpat(grp->root, i, dsize, ncap);

        if (child != NULL) {
            child->p = grp->root;
            child->pindex = i;
        }
    }

    // copy data to right node and ptrs to right node, and modify all children's p and pindex to right node's
    for (int i = 0; i < rl; ++i)
        fcopy(bdat(grp->r, i, dsize), bdat(bnd, ll + ml + i, dsize));

    memcpy(bdpat(grp->r, 0, dsize, ncap), bdpat(bnd, ll + ml, dsize, ncap), sizeof(BNode*) * (rl + 1));

    for (int i = 0; i <= rl; ++i) {
        BNode* child = *bdpat(grp->r, i, dsize, ncap);
        
        if (child != NULL) {
            child->p = grp->r;
            child->pindex = i;
        }
    }

    // clear left node's old ptrs
    memset(bdpat(grp->l, ll + 1, dsize, ncap), 0, sizeof(BNode*) * (ncap - ll));

    grp->l->nsize = ll;
    grp->root->nsize = ml;
    grp->r->nsize = rl;

    return ERROR(NULL, 0);
}

static Error nmerge(BGroup* grp, BNode* p, BTree* bt, FsAllocator* allocator) {
    while (p != NULL) {
        if (p->nsize + grp->root->nsize > bt->ncap) {
            int iroot = (p->nsize - 1) >> 1;
            BGroup ngrp;
            range grng[3] = {
                {
                    .begin = bdat(p, 0, bt->dsize),
                    .end = bdat(p, iroot - 1, bt->dsize),
                    .s = bt->dsize
                },
                {
                    .begin = bdat(p, iroot, bt->dsize),
                    .end = bdat(p, iroot, bt->dsize),
                    .s = bt->dsize
                },
                {
                    .begin = bdat(p, iroot + 1, bt->dsize),
                    .end = bdat(p, p->nsize - 1, bt->dsize),
                    .s = bt->dsize
                }
            };

            int rpindex = grp->root->pindex;

            Error error = n2grp(p, &ngrp, grng, bt->ncap, bt->fcopy, allocator);

            if (error.message != NULL) {
                return error;
            }

            bt->ncnt += 2;

            if (rpindex <= iroot) {
                nmerge(grp, ngrp.l, bt, allocator);
            } else {
                nmerge(grp, ngrp.r, bt, allocator);
            }

            grp->l = ngrp.l;
            grp->r = ngrp.r;
            grp->root = ngrp.root;
            p = grp->root->p;
        } else {
            int rpindex = grp->root->pindex;

            bshift(p, grp->root->pindex, grp->root->nsize, bt->dsize, bt->ncap, bt->fcopy);

            int pidt = grp->root->nsize;

            if (rpindex == p->nsize) {
                pidt = 0;
            }

            for (int i = 0; i < grp->root->nsize; ++i)
                bt->fcopy(bdat(p, grp->root->pindex + i - pidt, bt->dsize), bdat(grp->root, i, bt->dsize));
            for (int i = 0; i <= grp->root->nsize; ++i) {
                (*bdpat(grp->root, i, bt->dsize, bt->ncap))->pindex += grp->root->pindex - pidt;
                (*bdpat(grp->root, i, bt->dsize, bt->ncap))->p = p;
            }
            
            memcpy(bdpat(p, grp->root->pindex - pidt, bt->dsize, bt->ncap), bdpat(grp->root, 0, bt->dsize, bt->ncap), sizeof(BNode*) * (grp->root->nsize + 1));

            p->nsize += grp->root->nsize;

            return fsfree(allocator, (void**) &grp->root);
        }
    }

    bt->root = grp->root;

    return ERROR(NULL, 0);
}

static Result nfind(BTree* bt, void* data, CmpFn fcmp, int* index) {
    BNode* n = bt->root;

    while (n != NULL) {
        Error error = seq_ibsearch((range) { .begin = bdat(n, 0, bt->dsize), .end = bdat(n, n->nsize - 1, bt->dsize), .s = bt->dsize}, data, fcmp, index);

        if (error.message == NULL) {
            return RESULT_SUC(ptr, n);
        }

        n = *bdpat(n, *index, bt->dsize, bt->ncap);
    }

    return RESULT_FAIL("data not found for btree node", ECODE(TREELIB, BT, DNF));
}

Error btree_init(BTree* bt, int dsize, int ncap, CopyFn fcopy, CmpFn fcmp, SwapFn fswap) {
    if (bt == NULL || dsize <= 0 || ncap <= 2 || fcopy == NULL || fcmp == NULL || fswap == NULL) {
        return ERROR("bt == NULL or dsize <= 0 or ncap <= 2 or fcopy == NULL or fcmp == NULL or fswap == NULL", ECODE(TREELIB, BT, ARGV));
    }

    Error error = fsalloc_init(&bt->allocator, ndsize(dsize, ncap), 1, -1);

    if (error.message != NULL) {
        return error;
    }

    bt->dsize = dsize;
    bt->ncnt = 0;
    bt->ncap = ncap;
    bt->root = NULL;
    bt->fcopy = fcopy;
    bt->fcmp = fcmp;
    bt->fswap = fswap;

    return ERROR(NULL, 0);
}

Error btree_ainit(BTree* bt, int chunk_size, int pmin, int pmax, int ncap, CopyFn fcopy, CmpFn fcmp, SwapFn fswap) {
    if (bt == NULL || ncap <= 2 || fcopy == NULL || fcmp == NULL || fswap == NULL) {
        return ERROR("bt == NULL or ncap <= 2 or fcopy == NULL or fcmp == NULL or fswap == NULL", ECODE(TREELIB, BT, ARGV));
    }

    Error error = fsalloc_init(&bt->allocator, ndsize(chunk_size, ncap), pmin, pmax);

    if (error.message != NULL) {
        return error;
    }

    bt->dsize = chunk_size;
    bt->ncnt = 0;
    bt->ncap = ncap;
    bt->root = NULL;
    bt->fcopy = fcopy;
    bt->fcmp = fcmp;
    bt->fswap = fswap;

    return ERROR(NULL, 0);
}

Error btree_get(BTree* bt, void* dest) {
    if (bt == NULL || dest == NULL) {
        return ERROR("bt == NULL or dest == NULL", ECODE(TREELIB, BT, ARGV));
    }

    int did = 0;
    Result r = nfind(bt, dest, bt->fcmp, &did);

    if (r.has_value) {
        BNode* n = r.result.data.to_ptr;

        bt->fcopy(dest, bdat(n, did, bt->dsize));

        return ERROR(NULL, 0);
    }

    return ERROR("data not found for btree node", ECODE(TREELIB, BT, DNF));
}

Error btree_put(BTree* bt, void* src) {
    if (bt == NULL || src == NULL) {
        return ERROR("bt == NULL or src == NULL", ECODE(TREELIB, BT, ARGV));
    }

    int did = 0;
    Result r = nfind(bt, src, bt->fcmp, &did);

    if (r.has_value) {
        BNode* n = r.result.data.to_ptr;

        bt->fcopy(bdat(n, did, bt->dsize), src);

        return ERROR(NULL, 0);
    }

    return ERROR("data not found for btree node", ECODE(TREELIB, BT, DNF));
}

Error btree_repl(BTree* bt, void* dest, void* src) {
    if (bt == NULL || src == NULL || dest == NULL) {
        return ERROR("bt == NULL or src == NULL or dest == NULL", ECODE(TREELIB, BT, ARGV));
    }

    int did = 0;
    Result r = nfind(bt, dest, bt->fcmp, &did);

    if (r.has_value) {
        BNode* n = r.result.data.to_ptr;

        bt->fcopy(bdat(n, did, bt->dsize), src);

        return ERROR(NULL, 0);
    }

    return ERROR("data not found for btree node", ECODE(TREELIB, BT, DNF));
}

static void ptrcopy(void* d, void* s) {
    *(BNode**)d = *(BNode**)s;
}

Error btree_btrav(BTree* bt, VisFn fvisit) {
    if (bt == NULL || fvisit == NULL) {
        return ERROR("bt == NULL or ndata == NULL", ECODE(TREELIB, BT, ARGV));
    }

    if (bt->ncnt == 0) {
        return ERROR(NULL, 0);
    }

    Vec nvec;

    Error error = vec_init(&nvec, 5, sizeof(BNode*), ptrcopy);

    if (error.message != NULL) {
        return error;
    }

    BNode* n = bt->root;

    while (true) {
        fvisit(n);

        for (int i = n->nsize; i >= 0; --i) {
            if (*bdpat(n, i, bt->dsize, bt->ncap) != NULL) {
                error = vec_push(&nvec, bdpat(n, i, bt->dsize, bt->ncap));
            
                if (error.message != NULL) {
                    return error;
                }
            }
        }

        if (vec_pop(&nvec, &n).ec == ECODE(SEQLIB, VEC, EMPTY)) {
            break;
        }
    }

    return ERROR(NULL, 0);
}

Error btree_new(BTree* bt, void* ndata) {
    if (bt == NULL || ndata == NULL) {
        return ERROR("bt == NULL or ndata == NULL", ECODE(TREELIB, BT, ARGV));
    }

    if (bt->ncnt == 0) {
        Result nr = fsalloc(&bt->allocator);

        if (!nr.has_value) {
            return nr.result.error;
        }

        bt->root = nr.result.data.to_ptr;

        memset(bt->root, 0, ndsize(bt->dsize, bt->ncap));

        bt->root->p = NULL;
        bt->root->nsize = 1;
        bt->root->pindex = -1;
        bt->ncnt = 1;

        bt->fcopy(bdat(bt->root, 0, bt->dsize), ndata);

        return ERROR(NULL, 0);
    }

    int index = 0;
    BNode* n = bt->root;

    while (true) {
        Error error = seq_ibsearch((range) { .begin = bdat(n, 0, bt->dsize), .end = bdat(n, n->nsize - 1, bt->dsize), .s = bt->dsize}, ndata, bt->fcmp, &index);

        if (error.message == NULL) {
            return ERROR("duplicate data found in btree node", ECODE(TREELIB, BT, DUP));
        }

        if (*bdpat(n, index, bt->dsize, bt->ncap) == NULL) {
            if (n->nsize == bt->ncap) {
                int iroot = (n->nsize - 1) >> 1;
                BGroup grp;
                range grng[3] = {
                    {
                        .begin = bdat(n, 0, bt->dsize),
                        .end = bdat(n, iroot - 1, bt->dsize),
                        .s = bt->dsize
                    },
                    {
                        .begin = bdat(n, iroot, bt->dsize),
                        .end = bdat(n, iroot, bt->dsize),
                        .s = bt->dsize
                    },
                    {
                        .begin = bdat(n, iroot + 1, bt->dsize),
                        .end = bdat(n, n->nsize - 1, bt->dsize),
                        .s = bt->dsize
                    }
                };

                error = n2grp(n, &grp, grng, bt->ncap, bt->fcopy, &bt->allocator);

                if (error.message != NULL) {
                    return error;
                }

                bt->ncnt += 2;

                if (index <= iroot) {
                    bshift(grp.l, index, 1, bt->dsize, bt->ncap, bt->fcopy);
                    bt->fcopy(bdat(grp.l, index, bt->dsize), ndata);
                    grp.l->nsize += 1;
                } else {
                    bshift(grp.r, index - iroot - 1, 1, bt->dsize, bt->ncap, bt->fcopy);
                    bt->fcopy(bdat(grp.r, index - iroot - 1, bt->dsize), ndata);
                    grp.r->nsize += 1;
                }

                error = nmerge(&grp, grp.root->p, bt, &bt->allocator);

                if (error.message != NULL) {
                    return error;
                }
            } else {
                bshift(n, index, 1, bt->dsize, bt->ncap, bt->fcopy);
                bt->fcopy(bdat(n, index, bt->dsize), ndata);
                n->nsize += 1;
            }

            break;
        }

        n = *bdpat(n, index, bt->dsize, bt->ncap);
    }

    return ERROR(NULL, 0);
}

Error btree_del(BTree* bt, void* data);

Error btree_destroy(BTree* bt) {
    if (bt == NULL) {
        return ERROR("bt == NULL", ECODE(TREELIB, BT, ARGV));
    }

    Error error = fsalloc_destroy(&bt->allocator);

    if (error.message != NULL) {
        return error;
    }

    memset(bt, 0, sizeof(BTree));

    return ERROR(NULL, 0);
}