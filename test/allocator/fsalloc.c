#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fsalloc.h"

int main(int argc, char* argv[]) {
    char* ep;

    int chunk_size = strtol(argv[1], &ep, 10);
    int alloc_num = strtol(argv[2], &ep, 10);
    int pmax = strtol(argv[3], &ep, 10);
    int pmin = strtol(argv[4], &ep, 10);

    char* ptrs[alloc_num];

    printf("chunk size: %d, alloc num: %d, pmax: %d, pmin: %d\n", chunk_size, alloc_num, pmax, pmin);

    Error error;
    FsAllocator allocator;

    if ((error = fsalloc_init(&allocator, chunk_size, pmin, pmax)).message != NULL) {
        fprintf(stderr, "line %u, func %s, file %s: %s\n", error.line, error.function, error.file, error.message);
        return 1;
    }

    Result result;
    
    for (int ialloc = 0; ialloc < alloc_num; ++ialloc) {
        result = fsalloc(&allocator);

        if (result.has_value) {
            ptrs[ialloc] = result.result.data.to_ptr;
            printf("alloc pointer: %p\n", ptrs[ialloc]);
            memset(ptrs[ialloc], 'A', chunk_size);
        } else {
            fprintf(stderr, "line %u, func %s, file %s: %s\n", result.result.error.line, result.result.error.function, result.result.error.file, result.result.error.message);
            return 1;
        }
    }

    error = fsfree(&allocator, (void**) &ptrs[0]);

    if (error.message != NULL) {
        fprintf(stderr, "line %u, func %s, file %s: %s\n", error.line, error.function, error.file, error.message);
        return 1;
    }

    error = fsalloc_destroy(&allocator);

    if (error.message != NULL) {
        fprintf(stderr, "line %u, func %s, file %s: %s\n", error.line, error.function, error.file, error.message);
        return 1;
    }
}