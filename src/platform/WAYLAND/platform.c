#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_wayland.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>


#include <platform.h>
#include <render/render.h>
#include <render/pipeline.h>


#include "platform/wayland/wayland.h"
#include "platform/wayland/xdg_shell_protocol.h"
#include "render/memory.h"
#include "util.h"
#include "xdg_shell_protocol.c"

#include "listeners.c"
#include "services.c"


int allocFile(u32 size) {
    int fd = shm_open("SE_tmp_file", O_RDWR | O_CREAT | O_EXCL, 0600);
    assert(fd >= 0);
    shm_unlink("SE_tmp_file");
    ftruncate(fd, size);
    return fd;
}




const char* SE_PlatformInstanceExtensions[] = {
    VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME
};
const u32 SE_PlatformInstanceExtensionCount 
                    = ASIZE(SE_PlatformInstanceExtensions);

const char* title = "testapp";

int main(int argc, char* argv[]) {
    //set working directory

    char path[1024] = {0};
    if (readlink("/proc/self/exe", path, sizeof(path)) == -1) {
        perror("Failed to get Working Directory");
        SE_Exit(-1);
    }
    u32 end = sizeof(path) - 1;
    while (path[end] != '/') {
        path[end] = 0;
        end--;
    }

    if (chdir(path) == -1) {
        perror("Failed to set Working Directory");
        SE_Exit(-1);
    }

   SE_window w = {0}; 
   w.title = title;
   w.display = wl_display_connect(NULL);
   assert(w.display);

   SE_Log("Display Connected\n");

   w.registry = wl_display_get_registry(w.display);
   assert(w.registry);

   SE_Log("Registry Loaded\n");

   
   wl_registry_add_listener(w.registry, &SE_reg_handler, &w);
   wl_display_roundtrip(w.display);

   w.wsurf = wl_compositor_create_surface(w.wcomp);
   assert(w.wsurf);

   SE_Log("Wayland Surface Created\n");

   w.xsurf = xdg_wm_base_get_xdg_surface(w.xbase, w.wsurf);
   assert(w.xsurf);
   xdg_surface_add_listener(w.xsurf, &SE_xsurf_handler, &w);

   SE_Log("Xdg Surface Created\n");

   w.xtop = xdg_surface_get_toplevel(w.xsurf);
   assert(w.xtop);
   SE_Log("Xdg Toplevel Created\n");

   xdg_toplevel_set_title(w.xtop, title);
   xdg_toplevel_set_app_id(w.xtop, title);
   xdg_toplevel_add_listener(w.xtop, &SE_top_handler, &w);

   SE_Log("Title and AppID Set To: %s\n", title);


   //TODO(ELI): Remove shm when Vulkan Swapchain,
   //framebuffers are ready

   //temporary use shm to attach buffer
   //const int width = 800;
   //const int height = 600;
   //const int stride = width * 4;
   //const int size = stride * height;
   //int fd = allocFile(size);  
   //struct wl_shm_pool* pool = wl_shm_create_pool(w.shm, fd, size);
   //struct wl_buffer* buf = wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_XRGB8888);

   //void* data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
   //SE_memset(data, 128, size);
   //munmap(data, size);
 
   //wl_surface_commit(w.wsurf);

   //wl_surface_attach(w.wsurf, buf, 0, 0);
   //wl_surface_damage(w.wsurf, 0, 0, UINT32_MAX, UINT32_MAX);

   wl_surface_commit(w.wsurf);
   wl_display_roundtrip(w.display);
   wl_surface_commit(w.wsurf);

   //init Vulkan
   SE_render_context r = SE_CreateRenderContext(&w);
   SE_shaders s = SE_LoadShaders(&r, "shaders/vert.spv", "shaders/frag.spv");
   SE_render_pipeline p = SE_CreatePipeline(&r, &s);
   SE_render_memory mem = SE_CreateHeapTrackers(&r);
   SE_resource_arena v = SE_CreateResourceTrackerBuffer(&r, &mem,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, KB(1));



   //temporary loop
   while (!w.quit) {
       wl_display_roundtrip(w.display);
       SE_DrawFrame(&w, &r, &p);
   }

   return 0;
}

