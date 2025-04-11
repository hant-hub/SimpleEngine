#ifndef SE_GENERATED_STATIC_H
#define SE_GENERATED_STATIC_H

#include <util.h>
#include <stddef.h>
#include <util.h>
#include <generated/generated_types.h>

typedef struct SE_struct_member {
    SE_meta_type type; //enum, to be generated
    const char* name;
    u64 offset;
    u64 size;
} SE_struct_member;

#endif
