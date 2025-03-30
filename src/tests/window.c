#include "RenderContext.h"
#include "platform/platform.h"



int main(int argc, char* argv[]) {
    
    SE_window s = SE_CreateWindow(800, 600, "test");
    SE_RenderContext r = SE_CreateRenderContext(&s);

    SE_ShaderProg shader = SE_LoadShader(&r, "shaders/vert.spv", "shaders/frag.spv");
    CONSOLE_LOG("shaders compiled\n");

    while (!SE_WindowClose(&s)) {
        SE_WindowPoll(&s);
    }

    return 0;
}
