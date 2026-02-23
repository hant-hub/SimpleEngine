#include <graphics/graphics_intern.h>
#include <se_intern.h>

BufferAllocator InitBufferAllocator(SEwindow* w, VkBufferCreateInfo info, u32 props);
u32 SEConfigBufType(SEwindow* w, SEBufType bt, SEMemType mt, u64 size) {
    SEVulkan* v = GetGraphics(w);
    u32 props = 0;
    u32 queues[] = {v->queues.gfam, v->queues.tfam};

    VkBufferCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .flags = 0,
        .pQueueFamilyIndices = queues,
        .queueFamilyIndexCount = 1,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .usage = 0,
        .size = size,
    };

    switch (bt) {
        case SE_BUFFER_VERT:
            info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;
        case SE_BUFFER_INDEX:
            info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            break;
    }

    switch (mt) {
        case SE_MEM_STATIC:
            props = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            
            info.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            info.sharingMode = VK_SHARING_MODE_CONCURRENT;
            info.queueFamilyIndexCount = 1 + (v->queues.gfam != v->queues.tfam);

            break;
        case SE_MEM_DYNAMIC:
            props = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            break;
    }



    BufferAllocator a = InitBufferAllocator(w, info, props);
    dynPush(v->bufAllocators, a);
    return v->bufAllocators.size - 1;
}

BufferAllocator InitBufferAllocator(SEwindow* w, VkBufferCreateInfo info, u32 props) {
    SEVulkan* v = GetGraphics(w);

    VkBuffer buf;
    assert(!vkCreateBuffer(v->dev, &info, NULL, &buf));

    VkMemoryRequirements req;
    vkGetBufferMemoryRequirements(v->dev, buf, &req);

    for (int i = 0; i < v->memory.heaps.size; i++) {
        bool8 validMem = (req.memoryTypeBits & (1 << v->memory.types.data[i]));
        bool8 validProps = ((v->memory.props.data[i] & props) == props);

        if (validMem && validProps) {
            MemoryRange r = AllocateDeviceMem(&v->memory.heaps.data[i], req.size, req.alignment);
            vkBindBufferMemory(v->dev, buf, v->memory.mem.data[i], r.offset);

            BufferAllocator alloc = {
                .b = buf,
                .memid = i,
                .alignment = req.alignment,
                .r = r,
                .m = CreateManager(w->mem, r.size)
            };
            
            return alloc;
            break;
        }
    }

    debuglog("Failed to find appropriate allocator, maybe try Configuring the Max GPU Mem for this Memory");
    panic();
    return (BufferAllocator){0};
}

SEBuffer AllocBuffer(SEwindow* win, u32 bufID, u64 size) {
    SEVulkan* v = GetGraphics(win);

    BufferAllocator* allocator = &v->bufAllocators.data[bufID];
    SEBuffer out = {
        .parent = bufID,
        .r = AllocateDeviceMem(&allocator->m, size, allocator->alignment)
    };

    if (out.r.size < size) {
        debugerr("Failed to Alloc Buffer");
        panic();
    }

    return out;
}

void* GetHandle(SEwindow* win, SEBuffer b) {
    SEVulkan* v = GetGraphics(win);


    void* ptr;
    BufferAllocator alloc = v->bufAllocators.data[b.parent];
    vkMapMemory(v->dev, v->memory.mem.data[alloc.memid], b.r.offset, b.r.size, 0, &ptr);

    return ptr;
}

void FreeHandle(SEwindow* win, SEBuffer b, void* ptr) {
    SEVulkan* v = GetGraphics(win);

    BufferAllocator alloc = v->bufAllocators.data[b.parent];
    vkUnmapMemory(v->dev, v->memory.mem.data[alloc.memid]);
}
