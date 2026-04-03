#ifndef SE_X11_H
#define SE_X11_H

#include "se.h"
#include "se_intern.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <vulkan/vulkan_xlib.h>

typedef struct SEX11Window {
    //must stay at top of struct
    SEwindow win;
    SEVulkan graphics;
    Display* disp;
    Window x11win;        
} SEX11Window;

static const char* x11instanceExtensions[] = {
    SEinstanceExtensions,
    SEdebugInstanceExtensions,
    VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
};

static const char* x11validationLayers[] = {
    "VK_LAYER_KHRONOS_validation"
};



#endif
