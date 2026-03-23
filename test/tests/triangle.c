#include "se.h"
#include "core/cutils.h"
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

    SEStructSpec vertSpec[] = {
        (SEStructSpec){
            .name = sstring("x"),
            .offset = 0,
            .type = SE_VAR_TYPE_U32,
            .size = sizeof(u32),
        },
    };


    SEwindow* win = CreateWindow(GlobalAllocator, "test", &settings);

    SERenderPipelineInfo* r = SECreateRenderPipeline(win);
    u32 color = SEAddColorAttachment(win, r);
    u32 vbuf = SEAddVertexBuffer(win, r, SE_MEM_DYNAMIC, sizeof(u32) * 3);
    u32 layout = SEAddDescriptorLayout(win, r);

    u32 pipeConfig = SEAddPipeline(win, r, layout);
    SESetShaderFrag(win, r, pipeConfig, sstring("../shaders/basic.frag.spv"));
    SESetShaderVertex(win, r, pipeConfig, sstring("../shaders/basic.vert.spv"));
    SEAddVertexBinding(r, pipeConfig, SE_BINDING_VERTEX, vertSpec, ARRAY_SIZE(vertSpec));
    
    u32 color_pass = SENewPass(win, r);
    SEWriteColorAttachment(win, r, color_pass, color);
    SEUsePipeline(r, color_pass, pipeConfig);
    SEUseVertexBuffer(win, r, color_pass, vbuf);

    SESetBackBuffer(r, color);

    SERenderPipeline* pipe = SECompilePipeline(win, r);

    SESetViewPort(pipe, color_pass, 0.0f, 0.0f, 1.0f, 1.0f);
    SESetScissor(pipe, color_pass, 0.0f, 0.0f, 1.0f, 1.0f);

    u32* verts = SERetrieveDynBuf(win, pipe, vbuf);
    verts[0] = 0;
    verts[1] = 1;
    verts[2] = 2;

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
    while (!win->shouldClose) {
        Poll(win);

        if (win->keystate[KEY_ESC] == KEY_PRESSED) break;

        {
            static bool8 active = TRUE;
            if (win->keystate[KEY_A] == KEY_PRESSED && active) {
                verts[0] = (verts[0] + 1) % 3;
                verts[1] = (verts[1] + 1) % 3;
                verts[2] = (verts[2] + 1) % 3;
                active = FALSE;
            }
            if (win->keystate[KEY_A] == KEY_RELEASED) 
                active = TRUE;
        }

        {
            static bool8 active = TRUE;
            if (win->keystate[KEY_S] == KEY_PRESSED && active) {
                active = FALSE;
                SESetViewPort(pipe, color_pass, 0.0f, 0.0f, 1.0f, 1.0f);
                SESetScissor(pipe, color_pass, 0.0f, 0.0f, 1.0f, 0.5f);
            }
            if (win->keystate[KEY_S] == KEY_RELEASED) {
                SESetViewPort(pipe, color_pass, 0.0f, 0.0f, 1.0f, 1.0f);
                SESetScissor(pipe, color_pass, 0.0f, 0.0f, 1.0f, 1.0f);
                active = TRUE;
            }
        }

        SEExecutePipeline(win, pipe);

        clock_gettime(CLOCK_REALTIME, &next);
        double curr_time = (next.tv_sec - start.tv_sec) + ((double)(next.tv_nsec - start.tv_nsec))/(1000 * 1000 * 1000);
        frametime = 0.9 * frametime + 0.1 * curr_time;
        start = next;
    

        if (counter <= 0) {
            debuglog("Fps: %f", 1/frametime);

            counter = 6.0/12.0;
        }

        counter -= curr_time;
        
    }

    SEDestroyRenderPipelineInfo(win, r); 
    SEDestroyPipeline(win, pipe);
    DestroyWindow(win);
    return 0;
}
