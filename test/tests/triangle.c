#include "graphics/graphics_intern.h"
#include "se.h"
#include "cutils.h"


int main() {
    setdirExe();
    InitSE();
    SEwindow* win = CreateWindow(GlobalAllocator, "test");

    SEShader vert = AddShader(win, sstring("../shaders/basic.vert.spv"));
    SEShader frag = AddShader(win, sstring("../shaders/basic.frag.spv"));

    SELayout layout = SEAddLayout(win);

    SERenderPass* r = SECreateRenderPass(win);
    SEPipeline pipe = SEAddPipeline(win, r, layout, vert, frag);


    while (1) {
        Poll(win);

        BeginRender(win);
        u32 i = BeginRenderPass(win, r);

        DrawTriangle(win);

        EndRenderPass(win);
        EndRender(win, i);


        if (win->keystate[KEY_ESC] == KEY_PRESSED) break;
        if (win->keystate[KEY_A] == KEY_PRESSED) debuglog("bam");
        if (win->keystate[KEY_B] == KEY_PRESSED) debuglog("boom");
    }
    SEDestroyRenderPass(win, r);
    DestroyWindow(win);
    return 0;
}
