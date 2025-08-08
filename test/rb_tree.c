#include <stdio.h>
#include <stdlib.h>
#include "rb_tree.h"

static void ferr(Error* e) {
    if (e->message != NULL) {
        fprintf(stderr, "ecode %d: line %u, function %s, file %s for %s\n", e->ec, e->line, e->function, e->file, e->message);
        exit(1);
    }
}

static void i32copy(void* d, void* s) {
    *(int*)d = *(int*)s;
}

static order i32cmp(void* di, void* d) {
    int idi = *(int*)di;
    int id = *(int*)d;

    if (id < idi) {
        return left;
    } else if (id > idi) {
        return right;
    }

    return equal;
}

static void i32swap(void* a, void* b) {
    int s = *(int*)a;
    *(int*)a = *(int*)b;
    *(int*)b = s;
}

static void i32pvisit(void* d) {
    printf("%d ", *(int*)d);
}

int main(int argc, char* argv[]) {
    BasicTree rbt;
    Error error;
    char* ep;

    error = rb_tree_init(&rbt, sizeof(int), i32copy, i32cmp, i32swap);

    ferr(&error);

    for (int i = 1; i < argc; ++i) {
        int d = strtol(argv[i], &ep, 10);

        error = rb_tree_new(&rbt, &d);

        ferr(&error);
    }

    error = rb_tree_trav(&rbt, pre, i32pvisit);

    ferr(&error);

    printf("\n");

    error = rb_tree_trav(&rbt, in, i32pvisit);

    ferr(&error);

    printf("\n");

    error = rb_tree_trav(&rbt, post, i32pvisit);

    ferr(&error);

    printf("\n");

    int dd = strtol(argv[argc - 1], &ep, 10);

    error = rb_tree_del(&rbt, &dd);

    ferr(&error);

    error = rb_tree_trav(&rbt, pre, i32pvisit);

    ferr(&error);

    printf("\n");

    return 0;
}