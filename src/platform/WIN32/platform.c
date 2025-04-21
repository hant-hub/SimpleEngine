#include "render/render.h"
#include "util.h"
#include <platform.h>
#include <platform/win32/win32.h>
#include <stdlib.h>
#include "vulkan/vulkan_win32.h"

const char* SE_PlatformInstanceExtensions[] = {
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
};
const u32 SE_PlatformInstanceExtensionCount = ASIZE(SE_PlatformInstanceExtensions);

#ifdef SE_DEBUG_CONSOLE
void SE_printf(const char* format, ...) {
    static HANDLE console = INVALID_HANDLE_VALUE;

    if (console == INVALID_HANDLE_VALUE) {
        AllocConsole();
        console = GetStdHandle(STD_OUTPUT_HANDLE);
    }

    va_list args;
    va_start(args, format);
    WriteConsole(console, format, SE_strlen(format), NULL, NULL);
    va_end(args);
}
#endif

void* SE_HeapAlloc(u64 size) {
    return malloc(size);
}
void  SE_HeapFree(void* p) {
    free(p);
}

VkSurfaceKHR SE_CreateVKSurface(SE_window* win, VkInstance inst) {
    VkWin32SurfaceCreateInfoKHR surfInfo = {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .hinstance = win->instance,
        .hwnd = win->window,
    };

    VkSurfaceKHR surf;
    REQUIRE_ZERO(vkCreateWin32SurfaceKHR(inst, &surfInfo, NULL, &surf));
    return surf;
}

void SE_Exit(i32 i) {
    ExitProcess(i);
}

SE_string SE_LoadFile(const char* file) {
    return (SE_string){0};
}
void SE_UnloadFile(SE_string* file) {
}


static SE_window win = {0};

LRESULT CALLBACK WindowProc(HWND hwnd,
                            UINT uMsg,
                            WPARAM wParam,
                            LPARAM lParam) {
    LRESULT result = 0;
    switch (uMsg) {
        case WM_SIZE:
            {
                SE_Log("resize\n");
            } break;
        case WM_DESTROY:
            {
                SE_Log("destroy\n");
            } break;
        case WM_CLOSE:
            {
                SE_Log("close\n");
                DestroyWindow(hwnd);
                
            } break;
        case WM_ACTIVATEAPP:
            {
                SE_Log("active\n");
            } break;
        default:
            {
                result = DefWindowProc(hwnd, uMsg, wParam, lParam);
            } break;
    }
    return result;
}

int CALLBACK WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nShowCmd) {

    win.windowClass = (WNDCLASSA){
        .hInstance = hInstance,
        .lpfnWndProc = WindowProc,
        .lpszClassName = "SimpleEngineWindowClass",
        .style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW,
    };
    win.instance = hInstance;

    REQUIRE_ZERO(!RegisterClass(&win.windowClass));

    win.window = CreateWindowEx( 0, win.windowClass.lpszClassName, "Simple Engine",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT,
            0, 0, hInstance, 0);
    
    REQUIRE_ZERO(!win.window);
    MSG Message;
    BOOL result = GetMessage(&Message, 0, 0, 0);


    win.width = 800;
    win.height = 600;
    SE_render_context r = SE_CreateRenderContext(&win);


    while (1) {
        BOOL result = GetMessage(&Message, 0, 0, 0);
        if (result <= 0) break;
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }

    return 0;
}
