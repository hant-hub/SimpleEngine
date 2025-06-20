#ifndef SE_PLATFORM_H
#define SE_PLATFORM_H


#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <util.h>

#ifdef WAYLAND
#include "platform/wayland/wayland.h"
#elif defined(WIN32)
#include "platform/win32/win32.h"
#else
#error "Must Specify Platform"
typedef int SE_window;
#endif

#ifdef SE_DEBUG_CONSOLE
    void SE_printf(const char* format, ...);
    #define SE_Log(x, ...) SE_printf(__FILE__ ":%d]:\t" x, __LINE__,##__VA_ARGS__)
#else
    #define SE_Log(x, ...)
#endif

void* SE_HeapAlloc(u64 size);
void* SE_HeapRealloc(void* p, u64 size);
void  SE_HeapFree(void* p);

VkSurfaceKHR SE_CreateVKSurface(SE_window* win, VkInstance inst);

void SE_Exit(i32 i);

SE_string SE_LoadFile(const char* file);
void SE_UnloadFile(SE_string* file);

extern const char* SE_PlatformInstanceExtensions[];
extern const u32 SE_PlatformInstanceExtensionCount; 

#endif
