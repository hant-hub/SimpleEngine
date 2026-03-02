#include "graphics/graphics.h"
#include "se.h"
#include "cutils.h"
#include "vulkan/vulkan_core.h"

typedef struct SECmdBuf {
    VkCommandBuffer buf;
} SECmdBuf;

static VkBuffer* buf = NULL;

void temp(SECmdBuf* p, void* pass) {
    VkDeviceSize sizes[] = {0.0f};
    vkCmdBindVertexBuffers(p->buf, 0, 1, buf, sizes);
    vkCmdDraw(p->buf, 3, 1, 0, 0);
}

int main() {
    setdirExe();
    InitSE();
    SEwindow* win = CreateWindow(GlobalAllocator, "test", NULL);

    SERenderPipelineInfo* r = SECreatePipeline(win);

    u32 r1 = SEAddResource(r, TRUE);

    u32 vert = SEAddShader(win, r, sstring("../shaders/basic.vert.spv"));
    u32 vert2 = SEAddShader(win, r, sstring("../shaders/shift.vert.spv"));
    u32 frag = SEAddShader(win, r, sstring("../shaders/basic.frag.spv"));
    
    u32 layout = SEAddLayout(win, r);

    SEStructSpec vertSpec[] = {
        (SEStructSpec){
            .name = sstring("x"),
            .offset = 0,
            .type = SE_VAR_TYPE_U32,
            .size = sizeof(u32),
        },
    };

    SEBeginRenderPass(r);

    SEBindShaders(r, vert, frag, layout);
    SEAddDrawFunc(r, DrawTriangle);

    SEWriteResource(r, 0);

    SEEndRenderPass(r);
    SEBeginRenderPass(r);

    AddVertexBinding(r, SE_BINDING_INSTANCE, vertSpec, ARRAY_SIZE(vertSpec));

    SEBindShaders(r, vert2, frag, layout);
    SEAddDrawFunc(r, temp);

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
