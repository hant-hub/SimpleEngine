#ifndef SE_WIN32_H
#define SE_WIN32_H


#include <util.h>
#include <windows.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

typedef struct SE_window {
    const char* title;
    i32 width, height;
    Bool32 quit;
    Bool32 resize;

    HINSTANCE instance;
    WNDCLASSA windowClass;
    HWND window;
} SE_window;





#endif
