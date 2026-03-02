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
    SEConfigMaxGPUMem(win, SE_MEM_STATIC, KB(4));
    //u32 vert_alloc = SEConfigBufType(win, SE_BUFFER_VERT, SE_MEM_STATIC, KB(1));
    //SEBuffer vertex_buffer = AllocBuffer(win, vert_alloc, sizeof(u32) * 3);
    //u32* vert_handle = GetHandle(win, vertex_buffer);
    
    u32 data[][3] = {
        {0, 1, 2},
        {2, 0, 1},
        {1, 2, 0},
    };
    //CPUtoGPUBufferMemcpy(win, vertex_buffer, data[0], sizeof(data));

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

    //u32 vertbuf = SEAddVertexBuffer(r, vertex_buffer);

    SEBeginRenderPass(r);

    //SEReadResource(r, vertbuf);

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

            counter = 1.0/6.0;
            static u32 i = 0;
            //CPUtoGPUBufferMemcpy(win, vertex_buffer, data[i], sizeof(data));
            i = (i + 1) % 3;
        //    
        }

        counter -= curr_time;
        //usleep(1000 * 1000 * 0.3);
        
    }

    SEDestroyPipeline(win, p);
    SEDestroyPipelineInfo(win, r);
    DestroyWindow(win);
    return 0;
}
