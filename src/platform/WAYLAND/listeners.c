#include "platform/wayland/wayland.h"
#include "platform/wayland/xdg_shell_protocol.h"
#include <platform.h>
#include <util.h>
#include <wayland-client-protocol.h>

static Bool32 resizeReady = FALSE;

static void se_xdg_ping(void *data, struct xdg_wm_base *xdg_wm_base, 
        uint32_t serial) {
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener SE_xbase_handler = {
    .ping = se_xdg_ping
};

static void se_global_add(void *data, struct wl_registry *wl_registry,
               uint32_t name, const char *interface, 
               uint32_t version) {
    SE_window* win = data;
    if (SE_strcmp(interface, wl_compositor_interface.name)) {
        win->wcomp = wl_registry_bind(wl_registry, name, &wl_compositor_interface, 4);
    } else if (SE_strcmp(interface, xdg_wm_base_interface.name)) {
        win->xbase = wl_registry_bind(wl_registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(win->xbase, &SE_xbase_handler, NULL);
    }

    //temporary
    if (SE_strcmp(interface, wl_shm_interface.name)) {
        win->shm = wl_registry_bind(wl_registry, name, &wl_shm_interface, 1);
    }
}

static void se_global_remove(void *data, struct wl_registry *wl_registry,
                             uint32_t name) {
}

static const struct wl_registry_listener SE_reg_handler = {
    .global = se_global_add,
    .global_remove = se_global_remove,
};

static void SE_xsurf_configure(void *data, struct xdg_surface *xdg_surface,
			             uint32_t serial) {
    xdg_surface_ack_configure(xdg_surface, serial);

    if (resizeReady) {
        resizeReady = FALSE;
        SE_window* win = data;
        win->resize = TRUE;
    }
    SE_window* win = data;
    wl_surface_commit(win->wsurf);
}

static const struct xdg_surface_listener SE_xsurf_handler = {
    .configure = SE_xsurf_configure
};


static void SE_top_configure(void *data, struct xdg_toplevel *xdg_toplevel,
                         int32_t width, int32_t height,
                         struct wl_array *states) {

    if ((width > 0) && (height > 0)) {
        SE_Log("Resize: (%d, %d)\n", width, height);
        SE_window* win = data;
        if (win->width == width && win->height == height) return;
        win->width = width;
        win->height = height;
        resizeReady = TRUE;
    }
}

static void SE_top_close(void *data, struct xdg_toplevel *xdg_toplevel) {
    SE_window* win = data;
    win->quit = TRUE;
}

static void SE_top_configure_bounds(void *data, struct xdg_toplevel *xdg_toplevel,
				                int32_t width, int32_t height) {
}

static void SE_top_wm_capabilities(void *data, struct xdg_toplevel *xdg_toplevel,
				               struct wl_array *capabilities) {
}

static const struct xdg_toplevel_listener SE_top_handler = {
    .configure = SE_top_configure,
    .close = SE_top_close,
    .configure_bounds = SE_top_configure_bounds,
    .wm_capabilities = SE_top_wm_capabilities,
};
