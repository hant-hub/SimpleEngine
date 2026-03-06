#include "se.h"
#include "cutils.h"
#include <graphics/graphics_intern.h>
#include <vulkan/vulkan.h>
#include <time.h>
#include <unistd.h>



//void CustomDraw(SECmdBuf* cmd, void* handle) {
//    VkBuffer b = 0;
//    VkDeviceSize s = 0;
//    //vkCmdBindVertexBuffers(cmd->buf, 0, 1, &b, &s);
//    vkCmdDraw(cmd->buf, 3, 1, 0, 0);
//}


int main() {
    setdirExe();
    InitSE();
    SEsettings settings = {
        .memory = {
            .max_dynamic_mem = MB(30),
            .max_static_mem = MB(100),
        },
    };

    SEwindow* win = CreateWindow(GlobalAllocator, "test", &settings);

    SERenderPipelineInfo* r = SECreateRenderPipeline(win);
    u32 color = SEAddColorAttachment(win, r);

    u32 pipeConfig = SEAddPipeline(win, r);
    SESetShaderFrag(win, r, pipeConfig, sstring("../shaders/basic.frag.spv"));
    SESetShaderVertex(win, r, pipeConfig, sstring("../shaders/basic2.vert.spv"));
    
    u32 color_pass = SENewPass(win, r);
    SEWriteColorAttachment(win, r, color_pass, color);
    SEUsePipeline(r, color_pass, pipeConfig);

    SESetBackBuffer(r, color);

    SERenderPipeline* pipe = SECompilePipeline(win, r);


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


        if (win->keystate[KEY_ESC] == KEY_PRESSED) break;

        clock_gettime(CLOCK_REALTIME, &next);
        double curr_time = (next.tv_sec - start.tv_sec) + ((double)(next.tv_nsec - start.tv_nsec))/(1000 * 1000 * 1000);
        frametime = 0.999 * frametime + 0.001 * curr_time;
        start = next;


        if (counter <= 0) {
            debuglog("Fps: %f", 1/frametime);

            counter = 1.0/12.0;
        }

        counter -= curr_time;
        
    }

    DestroyWindow(win);
    return 0;
}
