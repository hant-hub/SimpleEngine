#include "ds.h"
#include "vulkan/vulkan_core.h"
#include <cutils.h>
#include <graphics/graphics_intern.h>


MemoryManager CreateManager(Allocator a, u32 idx, u32 flags, u32 size, SEVulkan* v) {
    MemoryManager m = {
        .freelist.a = a,
        .flags = flags,
    };
    //figure out heap to use

    VkMemoryAllocateInfo info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = size,
        .memoryTypeIndex = idx,
    };
    assert(vkAllocateMemory(v->dev, &info, NULL, &m.backingmem) == VK_SUCCESS);

    MemoryRange r = {
        .offset = 0,
        .size = size,
    };

    dynPush(m.freelist, r);

    return m;
}

MemoryRange AllocateDeviceMem(MemoryManager* m, u32 size) {

    MemoryRange r = {};

    //NOTE(ELI): Allocation math
    //start = ((offset + size - 1)/size) * size
    //size = size

    //scan freelist for range which is at least as large
    for (u32 i = 0; i < m->freelist.size; i++) {
        if (m->freelist.data[i].size < size) {
            continue;
        }
        if (m->freelist.data[i].size == size) {
            r = m->freelist.data[i];
            dynDel(m->freelist, i);
            break;
        }
        if (m->freelist.data[i].size > size) {
            r = m->freelist.data[i]; 
            MemoryRange fragment = {
                .offset = r.offset + size,
                .size = r.size - size,
            };
            m->freelist.data[i] = fragment;
            r.size = size;
            break;
        }
    }


    //return zero sized range if impossible to allocate
    return r;
}

void FreeDeviceMem(MemoryManager* m, MemoryRange r) {

    //insert into list in order
    //This will auto merge non fragmented blocks
    u32 idx = 0;
    for (u32 i = 0; i < m->freelist.size; i++) {
        if (m->freelist.data[i].offset > r.offset) {
            idx = i;
            break;
        }
    }

    //check adjacent blocks and merge if possible

    //merge into next
    MemoryRange post = m->freelist.data[idx];
    if (r.offset + r.size == post.offset) {
        r.size += post.size; 
        dynDel(m->freelist, idx); //delete
    }

    //merge into prev

    if (idx == 0) {
        dynIns(m->freelist, idx, r);
        return;
    }

    MemoryRange pre = m->freelist.data[idx - 1];
    if (r.offset == pre.offset + pre.size) {
        //replace previous node
        m->freelist.data[idx - 1].size += r.size;
    } else {
        //insert node
        dynIns(m->freelist, idx, r);
    }

}

void DestroyManager(SEVulkan* v, MemoryManager m) {
    //deallocate device memory

    vkFreeMemory(v->dev, m.backingmem, NULL);
    dynFree(m.freelist);
}
