#include "platform.h"
#include "util.h"
#include <render/render.h>
#include <render/memory.h>
#include <vulkan/vulkan_core.h>



SE_render_memory SE_CreateHeapTrackers(SE_render_context* r) {
    
    VkPhysicalDeviceMemoryProperties memprops;
    vkGetPhysicalDeviceMemoryProperties(r->p, &memprops);

    SE_render_memory m;
    m.heaps = SE_HeapAlloc(sizeof(SE_render_heap) * memprops.memoryHeapCount); 
    m.numheaps = memprops.memoryHeapCount;
    
    for (u32 i = 0; i < memprops.memoryHeapCount; i++) {
        m.heaps[i] = (SE_render_heap) {
            .size = (u64)memprops.memoryHeaps[i].size,
        };
    }

    for (u32 i = 0; i < memprops.memoryTypeCount; i++) {
        //coallate all memory props
        m.heaps[memprops.memoryTypes[i].heapIndex].props |= memprops.memoryTypes[i].propertyFlags;
    }

#ifdef SE_DEBUG_CONSOLE
    for (u32 i = 0; i < memprops.memoryHeapCount; i++) {
    SE_Log("Heap Tracked: %d, %ld, %.9b\n",
            i, m.heaps[i].size, m.heaps[i].props);
    }
#endif
    return m;
}

//tracker for Vertex Buffer
SE_resource_arena SE_CreateResourceTrackerBuffer(SE_render_context* r, SE_render_memory* m, VkBufferUsageFlagBits flags, VkMemoryPropertyFlagBits props, u64 minsize) {
    SE_resource_arena a;


    //calculate corrected size
    VkBufferCreateInfo bufInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = minsize,
        .usage = flags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    REQUIRE_ZERO(vkCreateBuffer(r->l, &bufInfo, NULL, (void*)&a.resource));
    SE_Log("Static Vertex Buffer Created\n");


    VkPhysicalDeviceMemoryProperties memprops;
    vkGetPhysicalDeviceMemoryProperties(r->p, &memprops);

    i32 heapidx = -1;
    for (u32 i = 0; i < m->numheaps; i++) {
        if (m->heaps[i].props & props) {
            heapidx = i;
            break;
        }
    }

    if (heapidx == -1) {
        SE_Log("Failed to find appropriate memory heap\n");
        SE_Exit(-1);
    }

    i32 typeidx = -1;
    for (u32 i = 0; i < memprops.memoryTypeCount; i++) {
        if (memprops.memoryTypes[i].propertyFlags & props) {
            typeidx = i;
            break;
        }
    }

    if (typeidx == -1) {
        SE_Log("Failed to find appropriate memory type\n");
        SE_Exit(-1);
    }


    VkMemoryRequirements memreq;
    vkGetBufferMemoryRequirements(r->l, a.resource, &memreq);

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .memoryTypeIndex = typeidx,
        .allocationSize = memreq.size,
    };

    REQUIRE_ZERO(vkAllocateMemory(r->l, &allocInfo, NULL, &a.devMem));
    a.size = memreq.size;
    a.alignment = memreq.alignment;


    SE_Log("Vertex Memory Allocated: Size %ld, Alignment %ld\n", a.size, a.alignment);

    VkBindBufferMemoryInfo bindInfo = {
        .sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO,
        .buffer = a.resource,
        .memory = a.devMem,
        .memoryOffset = 0,
    };

    REQUIRE_ZERO(vkBindBufferMemory(r->l, a.resource, a.devMem, 0));
    return a;
}
