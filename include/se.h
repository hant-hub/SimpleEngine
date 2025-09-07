#ifndef SE_H
#define SE_H

void CreateWindow();

#if defined(X11)
#include "platform/x11/x11.h"
#elif defined(WAYLAND)
#error "Wayland Not supported"
#elif defined(WIN32)
#error "Win32 Not Supported"
#else
#error "Application Background not Set"
#endif

#endif
