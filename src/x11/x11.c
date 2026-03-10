#include <se_intern.h>

#include "platform/x11/x11.h"
#include "cutils.h"
#include "se.h"
#include "strbase.h"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_xlib.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <graphics/graphics.h>

#include <unistd.h>

SEwindow *CreateWindow(Allocator a, const char *windowname, SEsettings* settings) {
    SEX11Window *win = Alloc(a, sizeof(SEX11Window));

    *win = (SEX11Window){
        .win =
            {
                .mem = a,
                .width = 800,
                .height = 600,
            },
        .disp = XOpenDisplay(NULL),
    };

    Window root = XDefaultRootWindow(win->disp);

    int BorderWidth = 0;
    int WindowDepth = CopyFromParent;
    int WindowClass = CopyFromParent;
    Visual *WindowVisual = CopyFromParent;

    int AttributeValueMask = CWBackPixel | CWEventMask;
    XSetWindowAttributes WindowAttributes = {};
    WindowAttributes.background_pixel = 0xff00ffff;
    WindowAttributes.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask | ExposureMask;

    win->x11win = XCreateWindow(win->disp, root, 0, 0, win->win.width, win->win.height, 0, WindowDepth, WindowClass,
                                WindowVisual, AttributeValueMask, &WindowAttributes);

    win->win.name = windowname;

    XSetStandardProperties(win->disp, win->x11win, win->win.name, win->win.name, None, NULL, 0, NULL);
    XMapWindow(win->disp, win->x11win);



    //graphics api Init

    SEVulkan* g = &win->graphics;
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = windowname,
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_3,
        .pEngineName = "Simple Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
    };

    VkDebugUtilsMessengerCreateInfoEXT messengerInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           //VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = &debugCallback,
        .pUserData = NULL
    };

    VkInstanceCreateInfo instInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = ARRAY_SIZE(x11instanceExtensions),
        .ppEnabledExtensionNames = x11instanceExtensions,


        #ifdef X11
        .enabledLayerCount = ARRAY_SIZE(x11validationLayers),
        .ppEnabledLayerNames = x11validationLayers,
        #endif

        .pNext = &messengerInfo 
    };

    vkCreateInstance(&instInfo, NULL, &g->inst);
    
    VkXlibSurfaceCreateInfoKHR surfInfo = {
        .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
        .dpy = win->disp,
        .window = win->x11win,
    };

    vkCreateXlibSurfaceKHR(g->inst, &surfInfo, NULL, &g->surf);
    CreateVulkan(g->inst, g->surf, (SEwindow *)win, &win->graphics, settings);

    return (SEwindow *)win;
}

SEVulkan* GetGraphics(SEwindow* win) {
    SEX11Window *handle = (SEX11Window*)win;
    return &handle->graphics;
}

void Poll(SEwindow *handle) {
    SEX11Window *win = (SEX11Window *)handle;

    u32 num_events = XEventsQueued(win->disp, QueuedAfterFlush);

    for (u32 i = 0; i < num_events; i++) {
        XEvent e = {};
        XNextEvent(win->disp, &e);

        switch (e.type) {
            case KeyPress:
            case KeyRelease: {
                XKeyEvent *press = (XKeyPressedEvent *)&e;

                KeySym key = XLookupKeysym(press, 0);

                u32 handled = 0;
                switch (key) {
                    case XK_Escape:
                        handle->keystate[KEY_ESC] = e.type == KeyPress ? KEY_PRESSED : KEY_RELEASED;
                        handled = 1;
                        break;

                    case XK_Super_L:
                        handle->keystate[KEY_SUPER] = e.type == KeyPress ? KEY_PRESSED : KEY_RELEASED;
                        handled = 1;
                        break;

                    case XK_Shift_L:
                        handle->keystate[KEY_LSHIFT] = e.type == KeyPress ? KEY_PRESSED : KEY_RELEASED;
                        handled = 1;
                        break;

                    case XK_Shift_R:
                        handle->keystate[KEY_RSHIFT] = e.type == KeyPress ? KEY_PRESSED : KEY_RELEASED;
                        handled = 1;
                        break;

                    default: break;
                }
                if (handled) break;


                //alphabet
                i32 test = key - XK_a;
                if (test >= 0 && test < 26) {
                    handle->keystate[KEY_A + test] = e.type == KeyPress ? KEY_PRESSED : KEY_RELEASED;
                    handled = 1;
                    break;
                }
                if (handled) break;

                //numbers
                test = key - XK_0;
                if (test >= 0 && test < 10) {
                    handle->keystate[KEY_0 + test] = e.type == KeyPress ? KEY_PRESSED : KEY_RELEASED;
                    handled = 1;
                    break;
                }
                if (handled) break;

                debugwarn("key %n not yet supported", XKeysymToString(key));
            } break;
            case ConfigureNotify:
            {
                XConfigureEvent* con = (XConfigureEvent *)&e; 
                //debuglog("Configure Window:");
                //debuglog("\tpos: (%d, %d)", con->x, con->y);
                //debuglog("\tsize: (%d, %d)", con->width, con->height);
                //debuglog("\tborder: %d", con->border_width);

                if (handle->width != con->width) {
                    handle->width = con->width;
                    handle->resize = TRUE;
                }

                if (handle->height != con->height) {
                    handle->height = con->height;
                    handle->resize = TRUE;
                }
            } break;

            case MapNotify:
            case Expose:
            {
                debuglog("Expose Event");
            } break;
            case ClientMessage:
            {
                XClientMessageEvent* msg = (XClientMessageEvent*)&e;
                if (msg->message_type == XInternAtom(win->disp, "WM_PROTOCOLS", 1)) {
                    if (msg->data.l[0] == XInternAtom(win->disp, "WM_DELETE_WINDOW", 0)) {
                        handle->shouldClose = 1;
                    } else {
                        char* name = XGetAtomName(win->disp, msg->data.l[0]);
                        debugwarn("Unhandled Protocol: %n", name);
                        XFree(name);
                    }
                } else {
                    char* name = XGetAtomName(win->disp, msg->message_type);
                    debugwarn("Unhandled Client Message Type: %n", name);
                    XFree(name);
                }
            } break;

            default: debugerr("Unsupported Event: %d", e.type); break;
        }
    }

}

void DestroyWindow(SEwindow *handle) {
    SEX11Window *win = (SEX11Window *)handle;


    SEVulkan* g = &win->graphics;

    DestroyVulkan(win->graphics, handle->mem);


    XDestroyWindow(win->disp, win->x11win);
    XCloseDisplay(win->disp);
    Free(handle->mem, handle, sizeof(SEX11Window));
    StrBaseFree(&stringbase);
}
