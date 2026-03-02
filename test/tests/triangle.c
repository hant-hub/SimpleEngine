#include "se.h"
#include "cutils.h"
#include <graphics/graphics_intern.h>
#include <vulkan/vulkan.h>
#include <time.h>
#include <unistd.h>



void CustomDraw(SECmdBuf* cmd, void* handle) {
    VkBuffer b = 0;
    VkDeviceSize s = 0;
    //vkCmdBindVertexBuffers(cmd->buf, 0, 1, &b, &s);
    vkCmdDraw(cmd->buf, 3, 1, 0, 0);
}


int main() {
    setdirExe();
    InitSE();
    SEwindow* win = CreateWindow(GlobalAllocator, "test", NULL);

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

    u32 vertbuf = SEAddVertexBuffer(r, SE_MEM_STATIC, sizeof(u32) * 3);
    SEBeginRenderPass(r);

    SEReadResource(r, vertbuf);

    AddVertexBinding(r, SE_BINDING_VERTEX, vertSpec, ARRAY_SIZE(vertSpec));
    SEBindShaders(r, vert, frag, layout);
    SEAddDrawFunc(r, CustomDraw);

    SEWriteResource(r, 0);

    SEEndRenderPass(r);

    SERenderPipeline* p = SECompilePipeline(win, r);
    u32* vert_handle = SEMapVertBuffer(win, p, vertbuf);
    assert(vert_handle);
    vert_handle[0] = 0;
    vert_handle[1] = 1;
    vert_handle[2] = 2;


    /*
        RenderPipeline r = CreateRenderPipeline(win);
        VertexBuf v = CreateVertexBuf(r, size, SE_VERT_USAGE_STATIC);
        SetVertexData(v, data);


        BeginRenderPass(r);
        AddColorTarget(r, p, SE_SCREEN);
        AddVertexBuf(r, p, v);
        EndRenderPass(r);
        
    */

    double counter = 0.2;
    double frametime = 1.0;
    struct timespec start, next;
    clock_gettime(CLOCK_REALTIME, &start);
    while (1) {
        Poll(win);

        SEDrawPipeline(win, p);

        if (win->keystate[KEY_ESC] == KEY_PRESSED) break;

        clock_gettime(CLOCK_REALTIME, &next);
        double curr_time = (next.tv_sec - start.tv_sec) + ((double)(next.tv_nsec - start.tv_nsec))/(1000 * 1000 * 1000);
        frametime = 0.999 * frametime + 0.001 * curr_time;
        start = next;


        if (counter <= 0) {
            debuglog("Fps: %f", 1/frametime);

            counter = 1.0/12.0;
            vert_handle[0] = (vert_handle[0] + 1) % 3;
            vert_handle[1] = (vert_handle[1] + 1) % 3;
            vert_handle[2] = (vert_handle[2] + 1) % 3;
        }

        counter -= curr_time;
        //usleep(1000 * 1000 * 0.3);
        
    }

    //FreeHandle(win, vertex_buffer, vert_handle);
    SEDestroyPipeline(win, p);
    SEDestroyPipelineInfo(win, r);
    DestroyWindow(win);
    return 0;
}
