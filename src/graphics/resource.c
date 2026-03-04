#include <graphics/graphics_intern.h>
#include <se_intern.h>

BufferAllocator InitBufferAllocator(SEwindow* w, VkBufferCreateInfo info, u32 props);
BufferAllocator SEConfigBufType(SEwindow* w, SEBufType bt, SEMemType mt, u64 size) {
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
    return a;
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

SEBuffer AllocBuffer(SEwindow* win, BufferAllocator* allocator, u32 bufID, u64 size) {
    SEVulkan* v = GetGraphics(win);

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


SEImage AllocImage(SEVulkan* v, VkImageUsageFlags usage, VkFormat* formats, u32 numFormats, u32 width, u32 height) {
    
    VkFormatFeatureFlags feat = 0;
    VkImageAspectFlags aspect = 0;
    if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
            feat |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
            aspect |= VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            feat |= VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
            aspect |= VK_IMAGE_ASPECT_DEPTH_BIT;
    }
     
    if (feat == 0) {
        debugerr("Usage Not yet Supported!");
        panic();
    }

    VkFormat selected = VK_FORMAT_UNDEFINED;
    for (u32 i = 0; i < numFormats; i++) {
        VkFormatProperties format_info;
        vkGetPhysicalDeviceFormatProperties(v->pdev, formats[i], &format_info);

        if ((format_info.optimalTilingFeatures & feat) == feat) {
            selected = formats[i];
            break;
        }
    }

    assert(selected != VK_FORMAT_UNDEFINED);
    debuglog("Selected Format: %d", selected);


    SEImage out = {
        .format = selected,
        .width = width,
        .height = height,
    }; 

    VkImageCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = out.format,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .sharingMode = (v->queues.gfam == v->queues.tfam) ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
        .extent = (VkExtent3D){width, height, 1},
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .arrayLayers = 1,
        .mipLevels = 1,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | usage,
        .queueFamilyIndexCount = 1 + (v->queues.gfam != v->queues.tfam),
        .pQueueFamilyIndices = (u32[]){v->queues.gfam, v->queues.tfam},
    };


    vkCreateImage(v->dev, &info, NULL, &out.img);

    VkMemoryRequirements req;
    vkGetImageMemoryRequirements(v->dev, out.img, &req);
    debuglog("Format: %d", out.format);
    debuglog("Img Req: Size %d, Alignment %d", req.size, req.alignment);

    for (int i = 0; i < v->memory.heaps.size; i++) {
        if (req.memoryTypeBits & (1 << v->memory.types.data[i])) {
            out.memid = i;
            out.r = AllocateDeviceMem(&v->memory.heaps.data[i], req.size, req.alignment);
            vkBindImageMemory(v->dev, out.img, v->memory.mem.data[i], out.r.offset);
            break;
        }
    }

    VkImageViewCreateInfo viewinfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .components = {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY,
        }, 
        .image = out.img,
        .format = out.format,
        .subresourceRange = {
            .aspectMask = aspect,
            .baseArrayLayer = 0,
            .layerCount = 1,
            .baseMipLevel = 0,
            .levelCount = 1,
        },
    };

    vkCreateImageView(v->dev, &viewinfo, NULL, &out.view);

    return out;
}

//void* GetHandle(SEwindow* win, SEBuffer b) {
//    SEVulkan* v = GetGraphics(win);
//
//    void* ptr;
//    BufferAllocator alloc = v->bufAllocators.data[b.parent];
//    vkMapMemory(v->dev, v->memory.mem.data[alloc.memid], b.r.offset, b.r.size, 0, &ptr);
//
//    return ptr;
//}
//
//void FreeHandle(SEwindow* win, SEBuffer b, void* ptr) {
//    SEVulkan* v = GetGraphics(win);
//
//    BufferAllocator alloc = v->bufAllocators.data[b.parent];
//    vkUnmapMemory(v->dev, v->memory.mem.data[alloc.memid]);
//}
