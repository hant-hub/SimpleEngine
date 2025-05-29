#ifndef SE_UTIL_H
#define SE_UTIL_H

#include <stdint.h>

typedef uint8_t  u8;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

typedef void*(*alloc_func)(u64);
typedef void(*free_func)(void*);


typedef struct SE_string {
    u64 size; 
    u8* data;
} SE_string;

//IDK why you'd need anything other than
//Bool32, but here they are, for fun
typedef u8 Bool8;
typedef u32 Bool32;
typedef u64 Bool64;

Bool32 SE_strcmp(const char* s1, const char* s2);
void SE_memcpy(void* dst, const void* src, u64 len);
void SE_memset(void* dst, u8 byte, u64 len);
int SE_strlen(const char* str);


#ifdef SE_ASSERT
#include <assert.h>

#define REQUIRE_ZERO(x) \
    { \
        i64 result = x; \
        if (result) {\
            SE_Log("%s : %d]: Failure: %d\n", __FILE__, __LINE__, result);\
            assert(0);\
            SE_Exit(-1);\
        } \
    }

#else
#define assert(x)

#define REQUIRE_ZERO(x) \
    if (x) {\
        SE_Exit(-1); \
    }
#endif

//Custom Allocator Interface -----------------------------
#define SE_alloc_func(x) \
    void* (x)(u64 oldsize, u64 newsize, void* ptr, void* ctx)

#define SE_mem_clean_func(x) \
    void (x)(void* ctx)

typedef SE_alloc_func(*SE_alloc_func);
typedef SE_mem_clean_func(*SE_mem_clean_func);

typedef struct SE_allocator {
    SE_alloc_func alloc; 
    void* ctx;
} SE_allocator;

//--------------------------------------------------------

typedef struct SE_mem_arena {
    u64 cap;
    u64 size;
    u8 data[];
} SE_mem_arena;

SE_alloc_func(SE_StaticArenaAlloc);
SE_mem_arena* SE_HeapArenaCreate(u64 size);

SE_alloc_func(SE_HeapGlobalAlloc);

#define ASIZE(x) (sizeof(x)/sizeof(x[0]))
#define TRUE 1
#define FALSE 0

#define CLAMP(max, min, val) \
    val <= max ? (val >= min ? val : min) : max

#define KB(x) (x * (1<<10))
#define MB(x) (x * (1<<20))
#define GB(x) (x * (1<<30))

#define META_INTROSPECT(x)
#define META_BASE(x)

#endif
