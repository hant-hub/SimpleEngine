#include "platform.h"
#include <util.h>


Bool32 SE_strcmp(const char* s1, const char* s2) {
    while (*s1 && *s2 && (*s1 == *s2)) {
        ++s1;
        ++s2;
    }
    return (*s1 == 0 && *s2 == 0);
}

void SE_memcpy(void* dst, const void* src, u64 len) {
    u8* dest = dst;
    const u8* source = src;
    for (u64 i = 0; i < len; i++) {
        dest[i] = source[i];
    }
}

void SE_memset(void* dst, u8 byte, u64 len) {
    u8* dest = dst;
    for (u64 i = 0; i < len; i++) {
        dest[i] = byte;
    }
}

int SE_strlen(const char* str) {
    const char* p = str;
    while (str[0]) str++;
    return str - p; 
}





SE_alloc_func(SE_StaticArenaAlloc) {
    SE_mem_arena* mem = ctx; 
    if (newsize == 0) {
        //free
        //no op for an arena
        return NULL;
    }

    //malloc
    if (mem->size + newsize > mem->cap) return NULL;
    void* new = &mem->data[mem->size];
    mem->size += newsize;

    if (ptr) {
        //realloc
        SE_memcpy(new, ptr, oldsize);
    }

    return new;
}

SE_mem_arena* SE_HeapArenaCreate(u64 size) {
    SE_mem_arena* m = SE_HeapAlloc(size + sizeof(SE_mem_arena));
    m->cap = size;
    m->size = 0;
    return m;
}
