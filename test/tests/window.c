#include "se.h"
#include "cutils.h"


int main() {
    InitSE();
    SEwindow* win = CreateWindow(GlobalAllocator, "test", NULL);
    while (!win->shouldClose){
        Poll(win);

        if (win->keystate[KEY_ESC] == KEY_PRESSED) break;
        if (win->keystate[KEY_A] == KEY_PRESSED) debuglog("bam");
        if (win->keystate[KEY_B] == KEY_PRESSED) debuglog("boom");
    }
    DestroyWindow(win);
    return 0;
}
