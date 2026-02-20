#include "se.h"
#include "cutils.h"
#include <graphics/graphics_intern.h>
#include <vulkan/vulkan.h>



void CustomDraw(SECmdBuf* cmd, void* handle) {
    VkBuffer b = 0;
    VkDeviceSize s = 0;
    //vkCmdBindVertexBuffers(cmd->buf, 0, 1, &b, &s);
    vkCmdDraw(cmd->buf, 3, 1, 0, 0);
}


int main() {
    setdirExe();
    InitSE();
    SEwindow* win = CreateWindow(GlobalAllocator, "test");
    SEConfigMaxGPUMem(win, SE_MEM_DYNAMIC, KB(4));
    u32 vert_alloc = SEConfigBufType(win, SE_BUFFER_VERT, SE_MEM_DYNAMIC, KB(1));
    SEBuffer vertex_buffer = AllocBuffer(win, vert_alloc, sizeof(u32) * 3);


    SERenderPipelineInfo* r = SECreatePipeline(win);

    SEStructSpec vertSpec[] = {
        (SEStructSpec){
            .name = sstring("x"),
            .offset = 0,
            .type = SE_VAR_TYPE_U32,
            .size = sizeof(u32),
        },
    };

    u32 vert = SEAddShader(win, r, sstring("../shaders/basic.vert.spv"));
    u32 vert2 = SEAddShader(win, r, sstring("../shaders/basic2.vert.spv"));
    u32 frag = SEAddShader(win, r, sstring("../shaders/basic.frag.spv"));
    
    u32 layout = SEAddLayout(win, r);

    u32 vertbuf = SEAddVertexBuffer(r, vertex_buffer);

    SEBeginRenderPass(r);

    SEReadResource(r, vertbuf);
    AddVertexBinding(r, SE_BINDING_VERTEX, vertSpec, ARRAY_SIZE(vertSpec));
    SEBindShaders(r, vert, frag, layout);
    SEAddDrawFunc(r, CustomDraw);

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
