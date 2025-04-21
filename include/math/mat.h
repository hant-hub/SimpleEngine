#ifndef SE_MAT_H
#define SE_MAT_H

#include "util.h"
#include <math/vector.h>

struct _SE_m2f {
    SE_v2f a; 
    SE_v2f b;
};

struct _SE_m3f {
    SE_v3f a; 
    SE_v3f b;
    SE_v3f c;
};

struct _SE_m4f {
    SE_v4f a; 
    SE_v4f b;
    SE_v4f c;
    SE_v4f d;
};

typedef union SE_m2f {
    struct _SE_m2f c;
    f32 a[4];
} SE_m2f;

typedef union SE_m3f {
    struct _SE_m3f c;
    f32 a[9];
} SE_m3f;

typedef union SE_m4f {
    struct _SE_m4f c;
    f32 a[16];
} SE_m4f;


SE_m2f SE_IdentityM2f(void);
SE_m3f SE_IdentityM3f(void);
SE_m4f SE_IdentityM4f(void);

SE_v2f SE_MatVec2f(SE_m2f m, SE_v2f v);
SE_v3f SE_MatVec3f(SE_m3f m, SE_v3f v); 
SE_v4f SE_MatVec4f(SE_m4f m, SE_v4f v); 

SE_m2f SE_MatMat2f(SE_m2f a, SE_m2f b);
SE_m3f SE_MatMat3f(SE_m3f a, SE_m3f b);
SE_m4f SE_MatMat4f(SE_m4f a, SE_m4f b);


#endif
