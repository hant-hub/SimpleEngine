#define _POSIX_C_SOURCE 200809L

#include "render/vertex.h"

#include <fcntl.h>
#include <dlfcn.h>
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

#include <user.h>

#include "math/mat.h"

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
    VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
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
    perror("dll load");

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

    wl_surface_commit(w.wsurf);
    wl_display_roundtrip(w.display);
    wl_surface_commit(w.wsurf);


    //init Vulkan
    SE api = (SE) {
        .w = &w,

            //render
            .LoadShaders = SE_LoadShaders,
            .CreateVertSpecInline = SE_CreateVertSpecInline,
            .CreateResourceTrackerBuffer = SE_CreateResourceTrackerBuffer,
            .CreateBuffer = SE_CreateBuffer,
            .TransferMemory = SE_TransferMemory,
            .InitPipelineCache = SE_InitPipelineCache,
            .PushPipelineType = SE_PushPipelineType,

            .AddShader = SE_AddShader,
            .AddVertSpec = SE_AddVertSpec,

            .FreePipelineCache = SE_FreePipelineCache,
            .BeginPipelineCreation = SE_BeginPipelineCreation,
            .OpqaueNoDepthPass = SE_OpqaueNoDepthPass,
            .EndPipelineCreation = SE_EndPipelineCreation,
            .CreateSyncObjs = SE_CreateSyncObjs,
            .DrawFrame = SE_DrawFrame,

            //memory
            .HeapAlloc = SE_HeapAlloc,
            .HeapRealloc = SE_HeapRealloc,
            .HeapFree = SE_HeapFree,

            .HeapArenaCreate = SE_HeapArenaCreate,
            .StaticArenaAlloc = SE_StaticArenaAlloc,
            .HeapGlobalAlloc = SE_HeapGlobalAlloc,
    };
    api.r = SE_CreateRenderContext(&w);

    SE_m4f i = SE_IdentityM4f();
    SE_m4f t = (SE_m4f){
        2, 0, 0, 0,
            0, 2, 0, 0,
            0, 0, 2, 0,
            0, 0, 0, 1,
    };

    SE_m4f out = SE_MatMat4f(i, t); 
    //SE_Log("Mat4: \t%f %f %f %f\n\t%f %f %f %f\n\t%f %f %f %f\n\t%f %f %f %f\n", 
    //        out.a[0], out.a[4], out.a[8],  out.a[12], 
    //        out.a[1], out.a[5], out.a[9],  out.a[13], 
    //        out.a[2], out.a[6], out.a[10], out.a[14], 
    //        out.a[3], out.a[7], out.a[11], out.a[15] 
    //      );


    void* game = dlopen("./init.so", RTLD_NOW);
    SE_user_state* user = dlsym(game, "state");
    api.user = *user;


    user->init(&api);
    while (!w.quit) {
        wl_display_roundtrip(w.display);
        user->draw(&api);
    }

    dlclose(game);
    return 0;
}

