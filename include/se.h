#ifndef SE_H
#define SE_H

#include "core/cutils.h"
#include "graphics/graphics.h"
#include <strbase/strbase.h>

#include <vulkan/vulkan.h>

extern StrBase stringbase;
typedef struct SEVulkan SEVulkan;

// keys
typedef enum KeyCodes {
    // letters
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,

    // Nums
    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,

    // arrows
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,

    // special keys
    KEY_ESC,
    KEY_TAB,
    KEY_LSHIFT,
    KEY_RSHIFT,
    KEY_CTRL,
    KEY_ALT,
    KEY_BACKSPACE,
    KEY_ENTER,
    KEY_SUPER,

    NUM_KEYS
} KeyCodes;

typedef enum KeyState {
    KEY_RELEASED,
    KEY_PRESSED,
} KeyState;

typedef struct SEsettings {
    struct {
        u32 max_static_mem;
        u32 max_dynamic_mem;
    } memory;

    struct {
        u32 width;
        u32 height;
    } size;
} SEsettings;

// should always be accessed via pointer
typedef struct SEwindow {
    Allocator mem;
    bool8 shouldClose;
    bool8 resize;
    u32 width, height;
    const char *name;
    KeyState keystate[NUM_KEYS];
} SEwindow;


void InitSE();
SEwindow *CreateWindow(Allocator a, const char *windowname, SEsettings* settings);
void Poll(SEwindow *handle);
void DestroyWindow(SEwindow *win);
SEVulkan *GetGraphics(SEwindow *win);

// #if defined(X11)
// #include "platform/x11/x11.h"
// #elif defined(WAYLAND)
// #error "Wayland Not supported"
// #elif defined(WIN32)
// #error "Win32 Not Supported"
// #else
// #error "Application Background not Set"
// #endif

#endif
