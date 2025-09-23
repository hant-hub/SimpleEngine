#include <se.h>

//important globals

StrBase stringbase = {};

void InitSE() {
    stringbase = (StrBase){GlobalAllocator};
}

