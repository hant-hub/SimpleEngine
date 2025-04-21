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

SE_mem_arena SE_ArenaCreateHeap(u32 size) {
    //align to 8 byte boundary
    size = (size + sizeof(u64)) & ~((u64)7);
    SE_mem_arena a = {
        .top = 0,
        .size = size,
        .data = SE_HeapAlloc(size)
    };

    return a;
}

SE_mem_arena SE_ArenaCreateArena(SE_mem_arena* a, u32 size) {
    size = (size + sizeof(u64)) & ~((u64)7);
    SE_mem_arena o = {
        .top = 0,
        .size = size,
        .data = SE_ArenaAlloc(a, size)
    };

    return o;
}

void SE_ArenaDestroyHeap(SE_mem_arena a) {
    SE_HeapFree(a.data);
}

void* SE_ArenaAlloc(SE_mem_arena* a, u64 size) {
    size = (size + sizeof(u64)) & ~((u64)7);
    
    if (size + a->top > a->size) {
        return 0;
    }

    void* out = &a->data[a->top];
    a->top += size;

    return out;
}

void SE_ArenaReset(SE_mem_arena* a) {
    a->top = 0;
}
