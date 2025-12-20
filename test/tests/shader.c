#include "graphics/graphics_intern.h"
#include "se.h"
#include "cutils.h"


int main() {
    setdirExe();
    InitSE();
    SEwindow* win = CreateWindow(GlobalAllocator, "test");
    SEShader shader = AddShader(win, sstring("../shaders/basic.vert.spv")); 


    DestroyWindow(win);
    return 0;
}
