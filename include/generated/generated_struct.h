#ifndef SE_GENERATED_DEF_H
#define SE_GENERATED_DEF_H
#include <math/vector.h>
#include <platform/wayland/wayland.h>
#include <util.h>
#include <generated/generated_static.h>
static SE_struct_member Meta_Def_vert[] = {
	{Meta_Type_SE_v2f, "pos", (u64)&(((struct vert *)0)->pos), (u64)sizeof(((struct vert *)0)->pos)},
	{Meta_Type_SE_v3f, "uv", (u64)&(((struct vert *)0)->uv), (u64)sizeof(((struct vert *)0)->uv)},
};
static SE_struct_member Meta_Def_SE_v4f[] = {
	{Meta_Type_float, "x", (u64)&(((struct SE_v4f *)0)->x), (u64)sizeof(((struct SE_v4f *)0)->x)},
	{Meta_Type_float, "y", (u64)&(((struct SE_v4f *)0)->y), (u64)sizeof(((struct SE_v4f *)0)->y)},
	{Meta_Type_float, "z", (u64)&(((struct SE_v4f *)0)->z), (u64)sizeof(((struct SE_v4f *)0)->z)},
	{Meta_Type_float, "w", (u64)&(((struct SE_v4f *)0)->w), (u64)sizeof(((struct SE_v4f *)0)->w)},
};
static SE_struct_member Meta_Def_SE_v3f[] = {
	{Meta_Type_float, "x", (u64)&(((struct SE_v3f *)0)->x), (u64)sizeof(((struct SE_v3f *)0)->x)},
	{Meta_Type_float, "y", (u64)&(((struct SE_v3f *)0)->y), (u64)sizeof(((struct SE_v3f *)0)->y)},
	{Meta_Type_float, "z", (u64)&(((struct SE_v3f *)0)->z), (u64)sizeof(((struct SE_v3f *)0)->z)},
};
static SE_struct_member Meta_Def_SE_v2f[] = {
	{Meta_Type_float, "x", (u64)&(((struct SE_v2f *)0)->x), (u64)sizeof(((struct SE_v2f *)0)->x)},
	{Meta_Type_float, "y", (u64)&(((struct SE_v2f *)0)->y), (u64)sizeof(((struct SE_v2f *)0)->y)},
};
#endif
