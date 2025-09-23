#ifndef SE_X11_H
#define SE_X11_h

#include "se.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

typedef struct SEX11Window {
    //must stay at top of struct
    SEwindow win;
    Display* disp;
    Window x11win;        
} SEX11Window;





#endif
