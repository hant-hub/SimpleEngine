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


static SE_struct_member Meta_Def_SE_v2f[] = {
    {Meta_Type_f32, "x", 0,           sizeof(f32)},
    {Meta_Type_f32, "y", sizeof(f32), sizeof(f32)},
};

static SE_struct_member Meta_Def_SE_v3f[] = {
    {Meta_Type_f32, "x", 0,               sizeof(f32)},
    {Meta_Type_f32, "y", sizeof(f32),     sizeof(f32)},
    {Meta_Type_f32, "z", sizeof(f32) * 2, sizeof(f32)},
};

static SE_struct_member Meta_Def_SE_v4f[] = {
    {Meta_Type_f32, "x", 0,               sizeof(f32)},
    {Meta_Type_f32, "y", sizeof(f32),     sizeof(f32)},
    {Meta_Type_f32, "z", sizeof(f32) * 2, sizeof(f32)},
    {Meta_Type_f32, "w", sizeof(f32) * 3, sizeof(f32)},
};


#endif
