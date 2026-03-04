#include "se.h"
#include "cutils.h"
#include <vulkan/vulkan.h>
#include <graphics/graphics_intern.h>

struct vert {
    f32 a;
    f32 b;
    f32 c;
} vert;

int main() {
    setdirExe();
    InitSE();
    SEsettings settings = {
        .memory.max_static_mem = MB(20),
    };
    SEwindow* win = CreateWindow(GlobalAllocator, "test", &settings);
    //SEConfigMaxGPUMem(win, SE_MEM_STATIC, MB(20));
    //u32 staticBufs = SEConfigBufType(win, SE_BUFFER_VERT, SE_MEM_DYNAMIC, MB(1));
    //SEBuffer buf = AllocBuffer(win, staticBufs, sizeof(vert) * 3);

    VkFormat swap = GetGraphics(win)->swapchain.format.format;
    VkFormat formats[] = {
        swap,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_FORMAT_B8G8R8A8_SRGB,
    };
    SEImage img = AllocImage(GetGraphics(win), VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, formats, ARRAY_SIZE(formats), 800, 600);  


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
