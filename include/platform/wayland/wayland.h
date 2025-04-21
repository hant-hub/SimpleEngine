#ifndef SE_WAYLAND_H
#define SE_WAYLAND_H

#include "math/vector.h"
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <platform/wayland/xdg_shell_protocol.h>

#include <util.h>

typedef struct SE_window {
    const char* title;
    i32 width, height;
    Bool32 quit;
    Bool32 resize;

    struct wl_display* display;
    struct wl_registry* registry;
    struct wl_compositor* wcomp;
    struct wl_surface* wsurf;

    struct xdg_wm_base* xbase;
    struct xdg_surface* xsurf;
    struct xdg_toplevel* xtop;

    //TODO(ELI): Remove Temporary buffer stuff
    struct wl_shm* shm;
} SE_window;

META_INTROSPECT() typedef struct vert {
    SE_v2f pos;
    SE_v3f uv;
} vert;


#endif
