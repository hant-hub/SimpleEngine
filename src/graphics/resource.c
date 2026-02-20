#include <graphics/graphics_intern.h>
#include <se_intern.h>

u32 SEConfigBufType(SEwindow* w, SEBufType bt, SEMemType mt, u64 size) {
    SEVulkan* v = GetGraphics(w);
    u32 usage = 0;
    u32 props = 0;

    switch (bt) {
        case SE_BUFFER_VERT:
            usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;
        case SE_BUFFER_INDEX:
            usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            break;
    }

    switch (mt) {
        case SE_MEM_STATIC:
            props = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            break;
        case SE_MEM_DYNAMIC:
            props = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            break;
    }


    BufferAllocator a = InitBufferAllocator(w, usage, props, size);
    dynPush(v->bufAllocators, a);
    return v->bufAllocators.size - 1;
}

BufferAllocator InitBufferAllocator(SEwindow* w, u32 usage, u32 props, u64 size) {
    SEVulkan* v = GetGraphics(w);

    VkBufferCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .flags = 0,
        .pQueueFamilyIndices = &v->queues.gfam,
        .queueFamilyIndexCount = 1,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .usage = usage,
        .size = size,
    };

    VkBuffer buf;
    assert(!vkCreateBuffer(v->dev, &info, NULL, &buf));

    VkMemoryRequirements req;
    vkGetBufferMemoryRequirements(v->dev, buf, &req);

    for (int i = 0; i < v->memory.heaps.size; i++) {
        if ((req.memoryTypeBits & (1 << v->memory.types.data[i])) &&
            ((v->memory.props.data[i] & props) == props)) {

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
