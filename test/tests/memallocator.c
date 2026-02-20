#include "graphics/graphics.h"
#include "graphics/graphics_intern.h"
#include "se.h"
#include "cutils.h"
#include <stddef.h>


int main() {
    setdirExe();
    InitSE();
    SEwindow* win = CreateWindow(GlobalAllocator, "test");

    //TODO(ELI): Change NULL to a real SEVulkan struct when
    //the manager is actually allocating
    MemoryManager m = CreateManager(GlobalAllocator, MB(2));

    printlog("initial:\n");
    for (u32 i = 0; i < m.freelist.size; i++) {
        printlog("\tRange: %d %d\n", m.freelist.data[i].offset, m.freelist.data[i].size);
    }

    MemoryRange r1 = AllocateDeviceMem(&m, 10, 2);
    MemoryRange r2 = AllocateDeviceMem(&m, 10, 2);
    MemoryRange r3 = AllocateDeviceMem(&m, 11, 2);
    MemoryRange r4 = AllocateDeviceMem(&m, 10, 2);

    printlog("after allocating:\n");
    for (u32 i = 0; i < m.freelist.size; i++) {
        printlog("\tRange: %d %d\n", m.freelist.data[i].offset, m.freelist.data[i].size);
    }

    FreeDeviceMem(&m, r1);

    printlog("unmergable free:\n");
    for (u32 i = 0; i < m.freelist.size; i++) {
        printlog("\tRange: %d %d\n", m.freelist.data[i].offset, m.freelist.data[i].size);
    }

    FreeDeviceMem(&m, r2);

    printlog("post merge free:\n");
    for (u32 i = 0; i < m.freelist.size; i++) {
        printlog("\tRange: %d %d\n", m.freelist.data[i].offset, m.freelist.data[i].size);
    }

    FreeDeviceMem(&m, r4);

    printlog("pre merge free:\n");
    for (u32 i = 0; i < m.freelist.size; i++) {
        printlog("\tRange: %d %d\n", m.freelist.data[i].offset, m.freelist.data[i].size);
    }

    FreeDeviceMem(&m, r3);

    printlog("double merge free:\n");
    for (u32 i = 0; i < m.freelist.size; i++) {
        printlog("\tRange: %d %d\n", m.freelist.data[i].offset, m.freelist.data[i].size);
    }

    DestroyManager(m);
    DestroyWindow(win);

    return 0;
}
