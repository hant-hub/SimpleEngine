#include "platform/x11/x11.h"
#include "X11/X.h"
#include "cutils.h"
#include "se.h"
#include "strbase.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <unistd.h>

SEwindow *CreateWindow(Allocator a, const char *windowname) {

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
    WindowAttributes.background_pixel = 0xffafe9af;
    WindowAttributes.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask | ExposureMask;

    win->x11win = XCreateWindow(win->disp, root, 0, 0, win->win.width, win->win.height, 0, WindowDepth, WindowClass,
                                WindowVisual, AttributeValueMask, &WindowAttributes);

    win->win.name = windowname;

    XSetStandardProperties(win->disp, win->x11win, win->win.name, win->win.name, None, NULL, 0, NULL);
    XMapWindow(win->disp, win->x11win);

    XFlush(win->disp);
    return (SEwindow *)win;
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

                if (press->keycode == XKeysymToKeycode(win->disp, XK_Escape)) {
                    handle->keystate[KEY_ESC] = e.type == KeyPress ? KEY_PRESSED : KEY_RELEASED;
                    break;
                }

                u32 handled = 0;
                for (u32 i = 0; i < 26; i++) {
                    if (press->keycode == XKeysymToKeycode(win->disp, XK_A + i)) {
                        handle->keystate[KEY_A + i] = e.type == KeyPress ? KEY_PRESSED : KEY_RELEASED;
                        handled = 1;
                        break;
                    }
                }
                if (handled) break;

                for (u32 i = 0; i < 10; i++) {
                    if (press->keycode == XKeysymToKeycode(win->disp, XK_0 + i)) {
                        handle->keystate[KEY_0 + i] = e.type == KeyPress ? KEY_PRESSED : KEY_RELEASED;
                        handled = 1;
                        break;
                    }
                }
                if (handled) break;

                debugwarn("keycode %d, %d not yet supported", press->keycode, 'a');
            } break;
            default: debugerr("Unsupported Event"); break;
        }
    }
}

void DestroyWindow(SEwindow *handle) {
    SEX11Window *win = (SEX11Window *)handle;

    XCloseDisplay(win->disp);
    XDestroyWindow(win->disp, win->x11win);
    Free(handle->mem, handle, sizeof(SEX11Window));
    StrBaseFree(&stringbase);
}
