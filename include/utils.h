#ifndef SE_UTILS_H
#define SE_UTILS_H
#include <stdint.h>

typedef uint32_t Bool32; 
#define TRUE 1
#define FALSE 0


typedef struct Buffer{
    uint64_t len;
    char* data;
} Buffer;

#define ASIZE(x) (sizeof(x)/sizeof(x[0]))

#define ZERO_CHECK(x) \
    if (x) SE_Exit(1)


#define CLAMP(max, min, val) \
    (val <= max ? (val >= min ? val : min) : max)


int strcmp(const char* s1, const char* s2);
void* memset(void* dst, int b, uint64_t size);



#endif
