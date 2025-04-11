#include <math/vector.h>


SE_v2f SE_Add2f(SE_v2f a, SE_v2f b) {
    return (SE_v2f){a.x + b.x, a.y + b.y};
}

SE_v3f SE_Add3f(SE_v3f a, SE_v3f b) {
    return (SE_v3f){a.x + b.x, a.y + b.y, a.z + b.z};
}

SE_v4f SE_Add4f(SE_v4f a, SE_v4f b) {
    return (SE_v4f){
        a.x + b.x,
        a.y + b.y,
        a.z + b.z,
        a.w + b.w,
    };
}


