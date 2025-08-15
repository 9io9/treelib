#include <stdio.h>
#include <stdlib.h>
#include "btree.h"

static void ferr(Error* e) {
    if (e->message != NULL) {
        fprintf(stderr, "ecode %d: line %u function %s file %s for %s\n", e->ec, e->line, e->function, e->file, e->message);
        exit(1);
    }
}

static void i32copy(void* d, void* s) {
    *(int*)d = *(int*)s;
}

static order i32cmp(void* a, void* b) {
    int ia = *(int*)a;
    int ib = *(int*)b;

    if (ia > ib) {
        return left;
    } else if (ia < ib) {
        return right;
    }

    return equal;
}

static inline void* at(void* base, int offset, int size) {
    return base + offset * size;
}

static inline void* bdat(BNode* bnd, int i, int dsize) {
    return at(bnd->ds, i, dsize);
}

static inline BNode** bdpat(BNode* bnd, int i, int dsize, int ncap) {
    return at(at(bnd->ds, ncap, dsize), i, sizeof(BNode*));
}

static void i32swap(void* a, void* b) {
    int swap = *(int*)a;
    *(int*)a = *(int*)b;
    *(int*)b = swap;
}

static int ncap;

static void i32ndpvisit(void* n) {
    BNode* nd = n;

    printf("node(%p, parent: %p, size: %d): ", nd, nd->p, nd->nsize);

    for (int i = 0; i < nd->nsize; ++i) {
        printf("%d ", *(int*)bdat(nd, i, sizeof(int)));
    }

    for (int i = 0; i <= nd->nsize; ++i) {
        printf("%p ", *bdpat(nd, i, sizeof(int), ncap));
    }

    printf("\n");
}

int main(int argc, char* argv[]) {
    char* ep;
    Error error;
    BTree bt;

    ncap = strtol(argv[1], &ep, 10);

    error = btree_init(&bt, sizeof(int), ncap, i32copy, i32cmp, i32swap);

    ferr(&error);

    for (int i = 2; i < argc; ++i) {
        int d = strtol(argv[i], &ep, 10);
        
        printf("%d\n", d);

        error = btree_new(&bt, &d);

        ferr(&error);
    }

    error = btree_btrav(&bt, i32ndpvisit);

    ferr(&error);

    int bd = 43;

    error = btree_get(&bt, &bd);

    ferr(&error);

    if (bd != 43) {
        fprintf(stderr, "btree get returns %d, but expect 43\n", bd);
        return 1;
    }

    return 0;
}