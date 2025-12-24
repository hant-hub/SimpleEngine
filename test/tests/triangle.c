#include "graphics/graphics_intern.h"
#include "se.h"
#include "cutils.h"


int main() {
    setdirExe();
    InitSE();
    SEwindow* win = CreateWindow(GlobalAllocator, "test");

    SERenderPipelineInfo* r = SECreatePipeline(win);

    u32 r1 = SEAddResource(r, TRUE);

    u32 vert = SEAddShader(win, r, sstring("../shaders/basic.vert.spv"));
    u32 frag = SEAddShader(win, r, sstring("../shaders/basic.frag.spv"));
    
    u32 layout = SEAddLayout(win, r);

    SEBeginRenderPass(r);

    SEBindShaders(r, vert, frag, layout);
    SEAddDrawFunc(r, DrawTriangle);

    SEWriteResource(r, 0);

    SEEndRenderPass(r);
    SEBeginRenderPass(r);

    SEBindShaders(r, vert, frag, layout);
    SEAddDrawFunc(r, DrawTriangle);

    SEWriteResource(r, 0);

    SEEndRenderPass(r);

    SERenderPipeline* p = SECompilePipeline(win, r);


    /*
        RenderPipeline r = CreateRenderPipeline(win);
        VertexBuf v = CreateVertexBuf(r, size, SE_VERT_USAGE_STATIC);
        SetVertexData(v, data);


        BeginRenderPass(r);
        AddColorTarget(r, p, SE_SCREEN);
        AddVertexBuf(r, p, v);
        EndRenderPass(r);
        
    */

    while (1){
        Poll(win);

        SEDrawPipeline(win, p);

        if (win->keystate[KEY_ESC] == KEY_PRESSED) break;
    }

    SEDestroyPipeline(win, p);
    SEDestroyPipelineInfo(win, r);
    DestroyWindow(win);
    return 0;
}
