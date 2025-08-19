#include <stdio.h>
#include <stdlib.h>
#include "trie.h"

static void i32copy(void* d, void* s) {
    *(int*)d = *(int*)s;
}

static void ferr(Error* e) {
    if (e->message != NULL) {
        fprintf(stderr, "ecode %d: line %u function %s file %s for %s\n", e->ec, e->line, e->function, e->file, e->message);
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    Trie trie;
    Error error;
    char* ep;

    error = trie_init(&trie, sizeof(int), i32copy);
    
    ferr(&error);

    for (int i = 1; i < argc; i += 2) {
        int d = strtol(argv[i + 1], &ep, 10);

        error = trie_put(&trie, argv[i], &d);
    
        ferr(&error);
    }

    int d = strtol(argv[argc - 1], &ep, 10);
    int pd = 0;

    error = trie_get(&trie, argv[argc - 2], &pd);

    ferr(&error);

    if (pd != d) {
        fprintf(stderr, "trie get returns (%s, %d), but expect (%s, %d)\n", argv[argc - 2], pd, argv[argc - 2], d);
        return 1;
    }

    error = trie_destroy(&trie);

    ferr(&error);

    return 0;
}