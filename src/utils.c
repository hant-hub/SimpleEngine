#include "utils.h"


int strcmp(const char* s1, const char* s2) {
    while (*s1 && *s2 && (*s1 == *s2)) {
        ++s1;
        ++s2;
    }
    return *s1 - *s2;
}

void* memset(void* dst, int b, uint64_t size) {
    char* buf = dst;
    for (uint64_t i = 0; i < size; i++) {
        buf[i] = b;
    }
    return dst;
}

