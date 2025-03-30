#include <vulkan/vulkan_core.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include "platform/platform.h"

#include <stdio.h>
#include <stdlib.h>

//reduce number of compilation units
#include "platform/wayland/xdg_shell_protocol.h"
#include "utils.h"
#include "xdg_shell_protocol.c"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_wayland.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

const char* InstanceExtensions[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
};


const uint32_t InstanceExtensionCount = ASIZE(InstanceExtensions);
const _exitproc SE_Exit = exit;

#ifdef SE_DEBUG_LOG
#include <stdarg.h>
void SE_Log(const char* s, ...) {
    va_list args;
    va_start(args, s);

    vprintf(s, args);

    va_end(args);
}
#endif

void* SE_Alloc(uint64_t size) {
    return malloc(size);
}

void SE_Free(void* p) {
    free(p);
}

Buffer SE_LoadFile(const char* path) {
    int fd = open(path, O_RDONLY);
    Buffer b = {0};  

    struct stat s;
    if (fstat(fd, &s)) {
        perror("Failed to Stat File");
        SE_Exit(1);
    }

    b.data = mmap(NULL, s.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    b.len = s.st_size;

    close(fd); //Why is this not in fnctl???
    return b;
}

void SE_UnLoadFile(Buffer* f) {
    munmap(f->data, f->len);
}

static void xdg_pong(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial) {
    xdg_wm_base_pong(xdg_wm_base, serial);
}


static void wl_global_remove(void *data, struct wl_registry *wl_registry,
                             uint32_t name) {
}


static Bool32 resizeRequest = FALSE;
static void xdg_surf_configure(void *data, struct xdg_surface *xdg_surface,
                         uint32_t serial) {

    xdg_surface_ack_configure(xdg_surface, serial);
    struct status* s = data;
    if (resizeRequest) {
        resizeRequest = FALSE;
        s->resize = TRUE;
    }
}

static void xdg_top_configure(void *data, struct xdg_toplevel *xdg_toplevel,
                              int32_t width, int32_t height, struct wl_array *states) {
    struct status* s = data;
    if (width && height) {
        resizeRequest = TRUE;
        s->height = height;
        s->width = width;
    }
}

static void xdg_close(void *data, struct xdg_toplevel *xdg_toplevel) {
    struct status* s = data;
    s->close = TRUE;
}

static void xdg_configure_bounds(void *data, struct xdg_toplevel *xdg_toplevel,
        int32_t width, int32_t height) {
}

static void xdg_wm_capabilities(void *data, struct xdg_toplevel *xdg_toplevel,
				                struct wl_array *capabilities) {
}

static const struct xdg_surface_listener xdg_surf_handler = {
    .configure = xdg_surf_configure,
};

static const struct xdg_toplevel_listener xdg_top_handler = {
    .configure = xdg_top_configure,
    .close = xdg_close,
    .configure_bounds = xdg_configure_bounds,
    .wm_capabilities = xdg_wm_capabilities,
};

static const struct xdg_wm_base_listener xdg_ping = {
    .ping = xdg_pong
};

static void wl_global(void *data, struct wl_registry *wl_registry,
        uint32_t name, const char *interface,
        uint32_t version) {
    SE_window* win = data;

    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        win->wl.c = wl_registry_bind(wl_registry, name, &wl_compositor_interface, 4);
    }

    if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        win->xdg.base = wl_registry_bind(wl_registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(win->xdg.base, &xdg_ping, NULL);
    }
}

static const struct wl_registry_listener wl_reg_handler = {
    .global = wl_global,
    .global_remove = wl_global_remove,
};


SE_window SE_CreateWindow(int width, int height, const char* title) {

    char bufpath[4096];
    int64_t len = readlink("/proc/self/exe", bufpath, sizeof(bufpath));
    ZERO_CHECK(len == -1);
    
    while (bufpath[len] != '/') {
        bufpath[len] = 0;
        len--;
    }

    if (chdir(bufpath) == -1) {
        perror("Failed to Set Directory");
        SE_Exit(1);
    }

    SE_window s = {0};
    s.wl.d = wl_display_connect(NULL);
    s.wl.r = wl_display_get_registry(s.wl.d);
    wl_registry_add_listener(s.wl.r, &wl_reg_handler, &s);
    wl_display_roundtrip(s.wl.d);

    s.wl.surf = wl_compositor_create_surface(s.wl.c);

    s.xdg.surf = xdg_wm_base_get_xdg_surface(s.xdg.base, s.wl.surf);
    xdg_surface_add_listener(s.xdg.surf, &xdg_surf_handler, &s.status);
    wl_surface_commit(s.wl.surf);
    wl_display_roundtrip(s.wl.d);

    s.status.width = width;
    s.status.height = height;

    return s;
}

Bool32 SE_WindowClose(SE_window* win) {
    return win->status.close;
}

void SE_WindowPoll(SE_window* win) {
    wl_display_roundtrip(win->wl.d);
}

Bool32 SE_CreateSurface(VkInstance instance, SE_window* win, VkSurfaceKHR* surf) {
    VkWaylandSurfaceCreateInfoKHR surfInfo = {
        .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .surface = win->wl.surf,
        .display = win->wl.d,
    };
    
    if (vkCreateWaylandSurfaceKHR(instance, &surfInfo, NULL, surf)) {
        return TRUE;
    }
    return FALSE;
}
