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
SE_resource_arena SE_CreateResourceTrackerBuffer(SE_render_context* r, VkBufferUsageFlagBits flags, VkMemoryPropertyFlagBits props, u64 minsize) {
    SE_resource_arena a = {0};
    SE_render_memory* m = &r->m;


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

SE_render_buffer SE_CreateBuffer(SE_resource_arena* a, u64 size) {

    if (a->top + size > a->size) {
        SE_Log("size: %d\n", a->top);
        SE_Log("Error, Not enough Memory!\n");
        return (SE_render_buffer){0};
    }

    SE_render_buffer buffer = (SE_render_buffer) {
        .offset = a->top,
        .size = size,
        .mem = a,
    };

    a->top += size;
    return buffer;
}


void SE_TransferMemory(SE_render_context* r, SE_render_buffer dst, void* data, u64 size) {
    SE_render_memory* m = &r->m;

    static SE_resource_arena staging_memory = {0};
    if (staging_memory.size == 0) {
        staging_memory = SE_CreateResourceTrackerBuffer(r, 
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                MB(1));
    }


    void* buffer;

    VkMappedMemoryRange mapping = (VkMappedMemoryRange) {
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .memory = staging_memory.devMem,
        .size = VK_WHOLE_SIZE,
        .offset = 0,
    };

    vkMapMemory(r->l, staging_memory.devMem, 0, VK_WHOLE_SIZE, 0, &buffer);
    SE_memcpy(buffer, data, size);
    vkFlushMappedMemoryRanges(r->l, 1, &mapping);
    vkUnmapMemory(r->l, staging_memory.devMem); 


    VkCommandBufferAllocateInfo allocinfo = (VkCommandBufferAllocateInfo){
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandBufferCount = 1,
        .commandPool = r->pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    };

    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(r->l, &allocinfo, &cmd);
    
    VkCommandBufferBeginInfo cmdbegin = (VkCommandBufferBeginInfo) {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    vkBeginCommandBuffer(cmd, &cmdbegin);

    VkBufferCopy copyInfo = (VkBufferCopy) {
        .dstOffset = dst.offset,
        .srcOffset = 0,
        .size = size,
    };
    vkCmdCopyBuffer(cmd, staging_memory.resource, dst.mem->resource, 1, &copyInfo);
    vkEndCommandBuffer(cmd);

    VkSubmitInfo subinfo = (VkSubmitInfo) {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
    };

    vkQueueSubmit(r->Queues.g, 1, &subinfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(r->Queues.g);

    vkFreeCommandBuffers(r->l, r->pool, 1, &cmd);
}

SE_resource_arena SE_CreateResourceTrackerRaw(SE_render_context* r, VkMemoryPropertyFlagBits props, u64 minsize) {
    SE_resource_arena a = {0};
    SE_render_memory* m = &r->m;

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

    //round up to nearest alignment size
    minsize = ((minsize/4096) + 1) * 4096;

    VkMemoryRequirements memreq = (VkMemoryRequirements) {
        .memoryTypeBits = props,
        .size = minsize,
        .alignment = 4096 //idk should be fine for now
    };

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .memoryTypeIndex = typeidx,
        .allocationSize = memreq.size,
    };

    REQUIRE_ZERO(vkAllocateMemory(r->l, &allocInfo, NULL, &a.devMem));
    a.size = memreq.size;
    a.alignment = memreq.alignment;

    return a;
}

u64 SE_CreateRaw(SE_resource_arena* a, u64 size) {
    if (a->top + size > a->size) {
        SE_Log("size: %d\n", a->top);
        SE_Log("Error, Not enough Memory!\n");
        return ~0;
    }
    u64 out = a->top;
    a->top += size;
    return out;
}
