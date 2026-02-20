#include <cutils.h>
#include <se.h>
#include <graphics/graphics_intern.h>

void SEConfigMaxGPUMem(SEwindow* win, SEMemType t, u64 size) {
    SEVulkan* v = GetGraphics(win);

    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(v->pdev, &memProps);

    u32 flags = 0;
    switch (t) {
        case SE_MEM_STATIC: 
            flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            break;
        case SE_MEM_DYNAMIC:
            flags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            break;
    }

    for (u32 i = 0; i < memProps.memoryTypeCount; i++) {
        VkMemoryType type = memProps.memoryTypes[i];

        if (type.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD) continue;
        if (type.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD) continue;

        if ((type.propertyFlags & flags) == flags) {
            MemoryManager m = CreateManager(win->mem, size);

            VkMemoryAllocateInfo info = {
                .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                .allocationSize = size,
                .memoryTypeIndex = i,
            };

            VkDeviceMemory mem;
            assert(vkAllocateMemory(v->dev, &info, NULL, &mem) == VK_SUCCESS);

            dynPush(v->memory.heaps, m);
            dynPush(v->memory.types, i);
            dynPush(v->memory.mem, mem);
            dynPush(v->memory.props, type.propertyFlags);
            break;
        }
    }
}


MemoryManager CreateManager(Allocator a, u32 size) {
    MemoryManager m = {
        .freelist.a = a,
    };
    //figure out heap to use


    MemoryRange r = {
        .offset = 0,
        .size = size,
    };

    dynPush(m.freelist, r);

    return m;
}

MemoryRange AllocateDeviceMem(MemoryManager* m, u32 size, u32 alignment) {

    MemoryRange r = {};

    //NOTE(ELI): Allocation math
    //start = ((offset + size - 1)/size) * size
    //size = size

    //scan freelist for range which is at least as large
    MemoryRange remainder;
    u32 idx = 0;
    for (u32 i = 0; i < m->freelist.size; i++) {
        MemoryRange freeRange = {
            .offset = ((m->freelist.data[i].offset + alignment - 1)/ alignment) * alignment,
            .size = m->freelist.data[i].size
        };

        freeRange.size = freeRange.size - (freeRange.offset - m->freelist.data[i].offset);

        remainder = (MemoryRange){
            .offset = m->freelist.data[i].offset,
            .size = (freeRange.offset - m->freelist.data[i].offset)
        };
        idx = i;

        if (freeRange.size < size) {
            continue;
        }
        if (freeRange.size == size) {
            r = m->freelist.data[i];
            dynDel(m->freelist, i);
            break;
        }
        if (freeRange.size > size) {
            r = freeRange; 
            MemoryRange fragment = {
                .offset = freeRange.offset + size,
                .size = freeRange.size - size,
            };
            m->freelist.data[i] = fragment;
            r.size = size;
            break;
        }
    }

    if (remainder.size) {
        dynIns(m->freelist, idx, remainder);
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

void DestroyManager(MemoryManager m) {
    //deallocate device memory
    dynFree(m.freelist);
}
