#include "se.h"
#include "cutils.h"
#include <vulkan/vulkan.h>

struct vert {
    f32 a;
    f32 b;
    f32 c;
} vert;

int main() {
    setdirExe();
    InitSE();
    SEwindow* win = CreateWindow(GlobalAllocator, "test");
    SEConfigMaxGPUMem(win, SE_MEM_DYNAMIC, MB(2));
    u32 staticBufs = SEConfigBufType(win, SE_BUFFER_VERT, SE_MEM_DYNAMIC, MB(1));
    SEBuffer buf = AllocBuffer(win, staticBufs, sizeof(vert) * 3);

    


    /* 
        Resource API

        SEShader s = CreateShader(win, vertpath, fragpath);
        SEImage i = CreateImage(win, format, size);
        
    */



    while (1){
        Poll(win);


        if (win->keystate[KEY_ESC] == KEY_PRESSED) break;
    }

    DestroyWindow(win);
    return 0;
}
