#ifndef SE_VECTOR_H
#define SE_VECTOR_H

#include <util.h>

struct _SE_v2f {
    f32 x;
    f32 y;
};

META_BASE()
typedef union SE_v2f {
    struct _SE_v2f c;
    f32 a[2];
} SE_v2f;

struct _SE_v3f {
    f32 x;
    f32 y;
    f32 z;
};

META_BASE()
typedef union SE_v3f {
    struct _SE_v3f c;
    f32 a[3];
} SE_v3f;

struct _SE_v4f {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
};

META_BASE()
typedef union SE_v4f {
    struct _SE_v4f c;
    f32 a[4];
} SE_v4f;


SE_v2f SE_Add2f(SE_v2f a, SE_v2f b);
SE_v3f SE_Add3f(SE_v3f a, SE_v3f b);
SE_v4f SE_Add4f(SE_v4f a, SE_v4f b);


f32 SE_Dot2f(SE_v2f a, SE_v2f b);
f32 SE_Dot3f(SE_v3f a, SE_v3f b);
f32 SE_Dot4f(SE_v4f a, SE_v4f b);
    
SE_v2f SE_Scale2f(SE_v2f v, f32 s);
SE_v3f SE_Scale3f(SE_v3f v, f32 s);
SE_v4f SE_Scale4f(SE_v4f v, f32 s);

#endif
