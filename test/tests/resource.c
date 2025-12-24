#include "graphics/graphics_intern.h"
#include "se.h"
#include "cutils.h"


int main() {
    setdirExe();
    InitSE();
    SEwindow* win = CreateWindow(GlobalAllocator, "test");


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
