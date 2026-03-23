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
            .type = SE_VAR_TYPE_V2F,
            .size = sizeof(v2f),
        },
    };


    SEwindow* win = CreateWindow(GlobalAllocator, "test", &settings);

    SERenderPipelineInfo* r = SECreateRenderPipeline(win);
    u32 color = SEAddColorAttachment(win, r);
    u32 vbuf = SEAddVertexBuffer(win, r, SE_MEM_DYNAMIC, sizeof(v2f) * 3);
    u32 ubuf = SEAddUniformBuffer(win, r, SE_MEM_DYNAMIC, 16 * 3);
    u32 layout = SEAddDescriptorLayout(win, r);

    u32 tex = SEAddTexture(win, r, SE_IMAGE_RGBA_8, 100, 100);

    SEAddDescriptorBinding(win, r, layout, SE_SHADER_VERTEX, SE_UNIFORM_BUFFER, 1);

    u32 pipeConfig = SEAddPipeline(win, r, layout);
    SESetShaderFrag(win, r, pipeConfig, sstring("../shaders/basic.frag.spv"));
    SESetShaderVertex(win, r, pipeConfig, sstring("../shaders/basic_vert.vert.spv"));
    SEAddVertexBinding(r, pipeConfig, SE_BINDING_VERTEX, vertSpec, ARRAY_SIZE(vertSpec));
    
    u32 color_pass = SENewPass(win, r);
    SEWriteColorAttachment(win, r, color_pass, color);
    SEUsePipeline(r, color_pass, pipeConfig);
    SEUseVertexBuffer(win, r, color_pass, vbuf);

    SESetBackBuffer(r, color);

    SERenderPipeline* pipe = SECompilePipeline(win, r);

    SESetViewPort(pipe, color_pass, 0.0f, 0.0f, 1.0f, 1.0f);
    SESetScissor(pipe, color_pass, 0.0f, 0.0f, 1.0f, 1.0f);

    SEBindUniformBuffer(win, pipe, color_pass, ubuf, 0);

    v2f* verts = SERetrieveDynBuf(win, pipe, vbuf);
    verts[0] = (v2f){0.0, -0.5};
    verts[1] = (v2f){0.5, 0.5};
    verts[2] = (v2f){-0.5, 0.5};
    SEUnmapDynBuf(win, pipe, vbuf);

    f32* uniforms = SERetrieveDynBuf(win, pipe, ubuf);
    uniforms[0] = 1.0f;
    uniforms[5] = 1.0f;
    uniforms[10] = 1.0f;
    SEUnmapDynBuf(win, pipe, ubuf);



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
