#include "X11/X.h"
#include "se.h"
#include "platform/x11/x11.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <unistd.h>


void CreateWindow() {
    Display* MainDisplay = XOpenDisplay(NULL);
    Window RootWindow = XDefaultRootWindow(MainDisplay);

    int WindowX = 0;
    int WindowY = 0;
    int WindowWidth = 800;
    int WindowHeight = 600;
    int BorderWidth = 0;
    int WindowDepth = CopyFromParent;
    int WindowClass = CopyFromParent;
    Visual* WindowVisual = CopyFromParent;

    int AttributeValueMask = CWBackPixel | CWEventMask;
    XSetWindowAttributes WindowAttributes = {};
    WindowAttributes.background_pixel = 0xffafe9af;
    WindowAttributes.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask | ExposureMask;

    Window MainWindow = XCreateWindow(MainDisplay, RootWindow, 
            WindowX, WindowY, WindowWidth, WindowHeight,
            BorderWidth, WindowDepth, WindowClass, WindowVisual,
            AttributeValueMask, &WindowAttributes);

    XMapWindow(MainDisplay, MainWindow);

    int quit = 0;
    while (!quit) {
        XEvent e = {};
        XNextEvent(MainDisplay, &e);

        switch (e.type) {
            case KeyPress:
            case KeyRelease:
                {
                    XKeyPressedEvent* event = (XKeyPressedEvent*)&e;
                    if (event->keycode == XKeysymToKeycode(MainDisplay, XK_Escape)) {
                        quit = 1;
                    }
                } break;
        }
    }
}
