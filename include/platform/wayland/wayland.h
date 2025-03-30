#ifndef WAYLAND_H
#define WAYLAND_H

#include <wayland-client.h>
#include <wayland-client-core.h>
#include "xdg_shell_protocol.h"
//TODO(ELI): Investigate removing libwayland
//dependency.

#include "utils.h"

typedef struct SE_window {
    struct status {
        int width, height;
        Bool32 close;
        Bool32 resize;
        //TODO(ELI): Test if Vulkan is all that is needed to 
        //resize the window, ie: VK_SWAPCHAIN_SUBOPTIMAL
    } status;

    struct wl {
        struct wl_display* d;
        struct wl_registry* r;
        struct wl_compositor* c;

        struct wl_surface* surf;
    } wl;

    struct xdg {
        struct xdg_wm_base* base;
        struct xdg_surface* surf;
        struct xdg_toplevel* top;
    } xdg;
} SE_window;




#endif
