/*
    fixed-size allocator:
        allocator for fixed size block, preallocate pages and divide them into fixed-sized chunk to allocate, and
        also have a free-list to reuse free chunks
*/

#include <stdlib.h>
#include <string.h>

#include "fsalloc.h"

#define FSALLOC_PAGE_SIZE (4096 * 2)
#define DEFAULT_PMIN 4

static void* alloc_page();
static void free_page(void* page);

static void* alloc_page() {
    return malloc(FSALLOC_PAGE_SIZE);
}

static void free_page(void* page) {
    if (page == NULL) {
        return;
    }

    free(page);
}


Error fsalloc_init(FsAllocator* allocator, int chunk_size, int pmin, int pmax) {
    if (allocator == NULL || chunk_size <= 0) {
        return ERROR("allocator == NULL or chunk_size <= 0");
    }

    if (pmin <= 0) {
        pmin = DEFAULT_PMIN;
    }

    allocator->pmax = pmax;
    allocator->pnum = pmin;
    allocator->slot_size = chunk_size + sizeof(FsFreeEntry);
    allocator->chunks_per_page = (FSALLOC_PAGE_SIZE - sizeof(FsPage)) / allocator->slot_size;
    allocator->page_list = alloc_page();
    allocator->free_list = NULL;
    
    if (allocator->page_list == NULL) {
        memset(allocator, 0, sizeof(FsAllocator));
        return ERROR("alloc_page return NULL, lack of memory");
    }

    allocator->cur_page = allocator->page_list;

    FsPage* p = allocator->page_list;

    for (int ip = 0; ip < pmin - 1; ++ip) {
        p->next_page = alloc_page();

        p->slots_free = allocator->chunks_per_page;

        if (p->next_page == NULL) {
            fsalloc_destroy(allocator);
            return ERROR("alloc_page return NULL, lack of memory");
        }

        p = p->next_page;
    }

    p->next_page = NULL;
    p->slots_free = allocator->chunks_per_page;

    return ERROR(NULL);
}

static inline void* at(void* base, int offset, int size) {
    return base + offset * size;
}

static inline void fck(char* check) {
    strcpy(check, "FSA");
    check[3] = '\0';
}

static inline bool check(char* check) {
    return strcmp(check, "FSA") == 0;
}

static inline void* frlalloc(FsFreeEntry** free_list) {
    void* e = (*free_list)->e;

    *free_list = (*free_list)->next_entry;

    return e;
}

static inline void frlfree(FsFreeEntry** free_list, FsFreeEntry* e) {
    e->next_entry = *free_list;

    *free_list = e;
}

Result fsalloc(FsAllocator* allocator) {
    if (allocator == NULL) {
        return RESULT_FAIL("allocator == NULL");
    }

    if (allocator->cur_page == NULL) {
        return RESULT_FAIL("allocator's page is NULL, allocator not init or init failed");
    }

    if (allocator->free_list != NULL) {
        return RESULT_SUC(ptr, frlalloc(&allocator->free_list));
    }

    if (allocator->cur_page->slots_free == 0) {
        if (allocator->pnum == allocator->pmax) {
            return RESULT_FAIL("reach pmax, no more memory should be allocated");
        }

        allocator->cur_page->next_page = alloc_page();

        if (allocator->cur_page->next_page == NULL) {
            return RESULT_FAIL("alloc_page return NULL, lack of memory");
        }

        allocator->cur_page = allocator->cur_page->next_page;
        allocator->cur_page->next_page = NULL;
        allocator->cur_page->slots_free = allocator->chunks_per_page;
        allocator->pnum += 1;
    }

    int islot = allocator->chunks_per_page - allocator->cur_page->slots_free;

    allocator->cur_page->slots_free -= 1;

    FsFreeEntry* ent = at(allocator->cur_page->entries, islot, allocator->slot_size);

    memset(ent, 0, allocator->slot_size);
    fck(ent->check);

    return RESULT_SUC(ptr, ent->e);
}

Error fsfree(FsAllocator* allocator, void** e) {
    if (e == NULL || *e == NULL || allocator == NULL) {
        return ERROR("e == NULL or *e == NULL or allocator == NULL");
    }

    FsFreeEntry* ent = *e - sizeof(FsFreeEntry);

    if (!check(ent->check)) {
        return ERROR("checksum failed, not allocated from fsalloc");
    }

    frlfree(&allocator->free_list, ent);

    *e = NULL;

    return ERROR(NULL);
}

Error fsalloc_destroy(FsAllocator* allocator) {
    if (allocator == NULL) {
        return ERROR("allocator == NULL");
    }

    if (allocator->page_list == NULL) {
        return ERROR("allocator has already been free");
    }

    while (allocator->page_list != NULL) {
        void* n = allocator->page_list->next_page;
        free_page(allocator->page_list);
        allocator->page_list = n;
    }

    memset(allocator, 0, sizeof(FsAllocator));
}

int fsalloc_psize() {
    return FSALLOC_PAGE_SIZE;
}

int fsalloc_pmin() {
    return DEFAULT_PMIN;
}