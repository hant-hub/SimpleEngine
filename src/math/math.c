#include <math/vector.h>
#include <math/mat.h>


//vector operations

SE_v2f SE_Add2f(SE_v2f a, SE_v2f b) {
    return (SE_v2f){a.c.x + b.c.x, a.c.y + b.c.y};
}

SE_v3f SE_Add3f(SE_v3f a, SE_v3f b) {
    return (SE_v3f){a.c.x + b.c.x, a.c.y + b.c.y, a.c.z + b.c.z};
}

SE_v4f SE_Add4f(SE_v4f a, SE_v4f b) {
    return (SE_v4f){
        a.c.x + b.c.x,
        a.c.y + b.c.y,
        a.c.z + b.c.z,
        a.c.w + b.c.w,
    };
}

f32 SE_Dot2f(SE_v2f a, SE_v2f b) {
    return a.c.x * b.c.x + a.c.y * b.c.y;
}

f32 SE_Dot3f(SE_v3f a, SE_v3f b) {
    return a.c.x * b.c.x + a.c.y * b.c.y + a.c.z * b.c.z;
}

f32 SE_Dot4f(SE_v4f a, SE_v4f b) {
    return 
        a.c.x * b.c.x +
        a.c.y * b.c.y +
        a.c.z * b.c.z +
        a.c.w * b.c.w;
}

SE_v2f SE_Scale2f(SE_v2f v, f32 s) {
    return (SE_v2f){
        v.c.x * s,
        v.c.y * s,
    };
}

SE_v3f SE_Scale3f(SE_v3f v, f32 s) {
    return (SE_v3f){
        v.c.x * s,
        v.c.y * s,
        v.c.z * s,
    };
}

SE_v4f SE_Scale4f(SE_v4f v, f32 s) {
    return (SE_v4f){
        v.c.x * s,
        v.c.y * s,
        v.c.z * s,
        v.c.w * s,
    };
}


//matrix operations

SE_m2f SE_IdentityM2f(void) {
    return (SE_m2f) {
        .a = {
            1, 0,
            0, 1
        }
    };
}

SE_m3f SE_IdentityM3f(void) {
    return (SE_m3f) {
        .a = {
            1, 0, 0,
            0, 1, 0,
            0, 0, 1
        }
    };
}

SE_m4f SE_IdentityM4f(void) {
    return (SE_m4f) {
        .a = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
        }
    };
}

SE_v2f SE_MatVec2f(SE_m2f m, SE_v2f v) {
    return (SE_v2f) {
        .a = {
            v.a[0] * m.a[0] + v.a[1] * m.a[2],
            v.a[0] * m.a[1] + v.a[1] * m.a[3],
        }
    };
}

SE_v3f SE_MatVec3f(SE_m3f m, SE_v3f v) {
    return (SE_v3f) {
        .a = {
            v.a[0] * m.a[0] + v.a[1] * m.a[3] + v.a[2] * m.a[6],
            v.a[0] * m.a[1] + v.a[1] * m.a[4] + v.a[2] * m.a[7],
            v.a[0] * m.a[2] + v.a[1] * m.a[5] + v.a[2] * m.a[8],
        }
    };
}

SE_v4f SE_MatVec4f(SE_m4f m, SE_v4f v) {
    return (SE_v4f) {
        .a = {
            v.a[0] * m.a[0] + v.a[1] * m.a[4] + v.a[2] * m.a[8]  + v.a[3] * m.a[12],
            v.a[0] * m.a[1] + v.a[1] * m.a[5] + v.a[2] * m.a[9]  + v.a[3] * m.a[13],
            v.a[0] * m.a[2] + v.a[1] * m.a[6] + v.a[2] * m.a[10] + v.a[3] * m.a[14],
            v.a[0] * m.a[3] + v.a[1] * m.a[7] + v.a[2] * m.a[11] + v.a[3] * m.a[15],
        }
    };
}

SE_m2f SE_MatMat2f(SE_m2f a, SE_m2f b) {
    return (SE_m2f){
        .c = {
            .a = SE_MatVec2f(a, b.c.a),
            .b = SE_MatVec2f(a, b.c.b),
        }
    };
}

SE_m3f SE_MatMat3f(SE_m3f a, SE_m3f b) {
    return (SE_m3f){
        .c = {
            .a = SE_MatVec3f(a, b.c.a),
            .b = SE_MatVec3f(a, b.c.b),
            .c = SE_MatVec3f(a, b.c.c),
        }
    };
}

SE_m4f SE_MatMat4f(SE_m4f a, SE_m4f b) {
    return (SE_m4f){
        .c = {
            .a = SE_MatVec4f(a, b.c.a),
            .b = SE_MatVec4f(a, b.c.b),
            .c = SE_MatVec4f(a, b.c.c),
            .d = SE_MatVec4f(a, b.c.d),
        }
    };
}
