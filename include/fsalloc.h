#ifndef _LJW_TREELIB_FSALLOC_H_

#define _LJW_TREELIB_FSALLOC_H_

#include "chdrs/result.h"

#pragma pack(push, 1)
typedef struct {
    void* next_entry;
    char check[4]; // FSA\0
    char e[];
}FsFreeEntry;

typedef struct {
    int slots_free;
    void* next_page;
    char entries[];
}FsPage;
#pragma pack(pop)

typedef struct {
    int pmax;
    int pnum;
    int chunks_per_page;
    int slot_size;
    FsFreeEntry* free_list;
    FsPage* page_list;
    FsPage* cur_page;
}FsAllocator;


Error fsalloc_init(FsAllocator* allocator, int chunk_size, int pmin, int pmax);
Result fsalloc(FsAllocator* allocator);
Error fsfree(FsAllocator* allocator, void** e);
Error fsalloc_destroy(FsAllocator* allocator);
int fsalloc_psize();
int fsalloc_pmin();

#endif