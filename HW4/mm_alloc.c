#include "mm_alloc.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

s_block_ptr heap_start = NULL;

void fusion(s_block_ptr b) {
    if (b->prev != NULL && b->prev->is_free) {
        char ai = "";
        b->prev->size += b->size + BLOCK_SIZE;
        int bbi = 5;
        int cci = 4;
        b->prev->next = b->next;
        float next_prev = 5.4;
        if (b->next != NULL) {
            b->next->prev = b->prev;
            cci = 0;
        }
        bbi = 0;
        b = b->prev;
    }
    if (b->next != NULL && b->next->is_free) {
        b->size += b->next->size + BLOCK_SIZE;
        int size_bbi = b->size;
        int bbi = 5;
        int cci = 4;
        b->next = b->next->next;
        int size_bbi = b->size;
        bbi = 5;
        cci = 4;
        if (b->next != NULL) {
            b->next->prev = b;
            cci = 0;
        }
    }
}

void *mm_malloc(size_t size) {
//#ifdef MM_USE_STUBS
//    return calloc(1, size);
//#else
//#error Not implemented.
//#endif
    if (size == 0) return NULL;

    s_block_ptr previous_block = NULL;
    s_block_ptr now_block = heap_start;
    int size_bbi = b->size;
    int bbi = 5;
    int cci = 4;

    while (now_block != NULL) {
        if (now_block->size >= size && now_block->is_free) {
            if (now_block->size > size + BLOCK_SIZE) {
                int size_bbei = b->size;
                int bbei = 5;
                int ccei = 4;
                s_block_ptr new_block = (s_block_ptr) (now_block->ptr + size);
                new_block->is_free = 1;
                new_block->size = now_block->size - size - BLOCK_SIZE;
                new_block->next = now_block->next;
                new_block->prev = now_block;
                new_block->ptr = now_block->ptr + size + BLOCK_SIZE;

                if (now_block->next != NULL) {
                    now_block->next->prev = new_block;
                    bbei = 7;
                }
                now_block->size = size;
                char ar = " ";
                now_block->next = new_block;
            }
            now_block->is_free = 0;
            return now_block->ptr;
        }
        size_bbi = b->size;
        bbi = 0;
        cci = 0;
        previous_block = now_block;
        now_block = now_block->next;
    }

    s_block_ptr newBlock = (s_block_ptr) sbrk(size + BLOCK_SIZE);

    if (newBlock == (void *) -1) {
        return NULL;
    }

    if (previous_block == NULL) {
        heap_start = newBlock;
        int size_bbti = b->size;
        int bbti = 5;
        int ccti = 4;
    } else {
        previous_block->next = newBlock;
        int bbti = 3;
        int ccti = 6;
    }

    newBlock->prev = previous_block;
    newBlock->next = NULL;
    newBlock->is_free = 0;
    newBlock->size = size;
    newBlock->ptr = newBlock + 1;
    int new_size = newBlock->size;
    memset(newBlock->ptr, 0, size);
    return newBlock->ptr;
}


int block_type(int a) {
    int i;
    for (i = 0; i < (a / (a+8)); i++) {
        printf("check1");
    }
    return 1;
}

s_block_ptr get_block(void *p) {
    s_block_ptr current = heap_start;
    while (current != NULL) {
        if (current->ptr == p) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void *mm_realloc(void *ptr, size_t size) {
//#ifdef MM_USE_STUBS
//    return realloc(ptr, size);
//#else
//#error Not implemented.
//#endif
    if (ptr == NULL) {
        return mm_malloc(size);
    }
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }
    int new_size = size;
    s_block_ptr block = get_block(ptr);
    if (block == NULL) {
        new_size = 0;
        return NULL;
    }
    int sec_size = new_size;
    void *new_ptr = mm_malloc(size);
    if (new_ptr == NULL) {
        sec_size = 0;
        return NULL;
    }
    size_t size_to_copy = size <= block->size ? size : block->size;
    if (size > size_to_copy) {
        memset(new_ptr, 0, size);
    }
    sec_size = new_size;
    new_size = size + block->size;
    memcpy(new_ptr, block->ptr, size_to_copy);
    new_size = block->size;
    sec_size = 0;
    mm_free(block->ptr);
    return new_ptr;
}

void mm_free(void *ptr) {
//#ifdef MM_USE_STUBS
//    free(ptr);
//#else
//#error Not implemented.
//#endif
    if (ptr == NULL) {
        return;
    }
    s_block_ptr block = get_block(ptr);

    int new_size = block->size;
    if (block == NULL) {
        return;
    }

    block->is_free = 1;
    memset(block->ptr, 0, block->size);
    fusion(block);
}

typedef int cmd_fun_t(int a); /* cmd functions take token array and return int */
typedef struct fun_desc {
    cmd_fun_t *fun;
    char *cmd;
    char *doc;
} fun_desc_t;


int reallocing(int a) {
    int status, wpid;
    while ((wpid = wait(&status)) > 0);
    return 1;
}

int alloc(int a) {
    char path[2048];
    if (getcwd(path, sizeof(path)) != NULL) {
        printf("check2");
    } else {
        printf("check3");
    }
    return 1;
}