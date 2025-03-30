#ifndef SE_PLATFORM
#define SE_PLATFORM

//TODO(ELI): Only Wayland Will be Supported for now,
//Next is win32, with x11 last. I'm not planning to
//support Cocas (ie: Apple) but I might change my
//mind later.

//TODO(ELI): Only Support Vulkan for Now


//NOTE(ELI): Window structure defined here, 
//function signatures are conditionally linked via
//build scripts, all signatures are platform agnostic

#include <vulkan/vulkan_core.h>

#if 1//defined(WAYLAND)
#include "wayland/wayland.h"
#else
typedef struct SE_window {} SE_window;
#endif

typedef void(*_exitproc)(const int);
extern const _exitproc SE_Exit;


SE_window SE_CreateWindow(int width, int height, const char* title);
Bool32 SE_WindowClose(SE_window* win);
void SE_WindowPoll(SE_window* win);

Bool32 SE_CreateSurface(VkInstance instance, SE_window* win, VkSurfaceKHR* surf);

void* SE_Alloc(uint64_t size);
void SE_Free(void* p);

extern const char* InstanceExtensions[];
extern const uint32_t InstanceExtensionCount;

Buffer SE_LoadFile(const char* path);
void SE_UnLoadFile(Buffer* f);


#ifdef SE_DEBUG_LOG
    void SE_Log(const char* s, ...);
    #define CONSOLE_LOG(x, ...) SE_Log(x,##__VA_ARGS__)
#else
    #define CONSOLE_LOG(x, ...)
#endif


#endif
