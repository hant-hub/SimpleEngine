#include "util.h"
#include <platform.h>
#include <stdlib.h>
#include <stdio.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>

#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_wayland.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

void* SE_HeapAlloc(u64 size) {
    return malloc(size);
}

void* SE_HeapRealloc(void* p, u64 size) {
    return realloc(p, size);
}

void  SE_HeapFree(void* p) {
    free(p);
}

void SE_Exit(i32 i) {
    exit(i);
}

VkSurfaceKHR SE_CreateVKSurface(SE_window* win, VkInstance inst) {
    VkWaylandSurfaceCreateInfoKHR surfInfo = {
        .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .display = win->display,
        .surface = win->wsurf,
    };
    VkSurfaceKHR surf;
    REQUIRE_ZERO(vkCreateWaylandSurfaceKHR(inst, &surfInfo, NULL, &surf));
    return surf;
}

#ifdef SE_DEBUG_CONSOLE
void SE_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
#endif

//mmapped io
SE_string SE_LoadFile(const char* file) {
    SE_string s;

    int fd = open(file, O_RDONLY);    
    assert(fd);
    
    struct stat fileinfo;
    if (fstat(fd, &fileinfo)) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "Failed Stat: %s", file);
        perror(buffer);
        SE_Exit(-1);
    }

    s.data = mmap(NULL, fileinfo.st_size, PROT_READ, MAP_PRIVATE, fd, 0); 
    s.size = fileinfo.st_size;

    close(fd);
    return s;
}

void SE_UnloadFile(SE_string* file) {
    munmap(file->data, file->size);
}
