#include <graphics/graphics_intern.h>

SERenderPipelineInfo *SECreateRenderPipeline(SEwindow *win) {
    SEVulkan *v = GetGraphics(win);
    SERenderPipelineInfo *r = Alloc(win->mem, sizeof(SERenderPipelineInfo));
    *r = (SERenderPipelineInfo){
        .passes.a = win->mem,
        .resources.a = win->mem,
        .pipeline.a = win->mem,
        .layouts.a = win->mem,
        .samplers.a = win->mem,
    };

    return r;
}

u32 SEAddTextureSampler(SEwindow *win, SERenderPipelineInfo *r) {
    SEVulkan *v = GetGraphics(win);
    dynPush(r->samplers, (VkSamplerCreateInfo){0});
    dynBack(r->samplers) = (VkSamplerCreateInfo){
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,

        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,

        .anisotropyEnable = v->features.anisotropy,
        .maxAnisotropy = 16 > v->featureInfo.anisotropy.max ? v->featureInfo.anisotropy.max : 16,

        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,

        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,

        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .mipLodBias = 0.0f,
        .minLod = 0.0f,
        .maxLod = 0.0f,
    };

    return r->samplers.size - 1;
}

u32 SEAddDescriptorLayout(SEwindow *win, SERenderPipelineInfo *r) {
    DescriptorLayout layout = {
        .info = (VkDescriptorSetLayoutCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        },
        .bindings.a = win->mem,
    };

    dynPush(r->layouts, layout);
    return r->layouts.size - 1;
}

void SEAddDescriptorBinding(SEwindow *win, SERenderPipelineInfo *r, u32 layout, SEShaderStage stage,
                            SEDescriptorType type, u32 count) {

    DescriptorLayout *handle = &r->layouts.data[layout];

    VkDescriptorSetLayoutBinding binding = {
        .binding = handle->bindings.size,
        .descriptorCount = count,
        .pImmutableSamplers = NULL,
    };

    switch (type) {
        case SE_TEXTURE_2D: binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; break;
        case SE_UNIFORM_BUFFER: binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; break;
    }

    if (stage & SE_SHADER_VERTEX)
        binding.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
    if (stage & SE_SHADER_FRAGMENT)
        binding.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;

    dynPush(handle->bindings, binding);
}

u32 SEAddColorAttachment(SEwindow *win, SERenderPipelineInfo *r) {
    SEVulkan *v = GetGraphics(win);
    dynPush(r->resources, (Resource){0});
    dynBack(r->resources) = (Resource){
      .type = RESOURCE_IMAGE,
      .resourceInfo.img =
          {
              .width = 1.0,
              .height = 1.0,
              .swapRel = TRUE,
              .persist = FALSE,
              .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
              .format = v->swapchain.format.format,
              .layout = VK_IMAGE_LAYOUT_UNDEFINED,
          },
  };

    return r->resources.size - 1;
}

u32 SEAddVertexBuffer(SEwindow *win, SERenderPipelineInfo *r, SEMemType t, u32 size) {
    dynPush(r->resources, (Resource){0});
    dynBack(r->resources) = (Resource) {
        .type = RESOURCE_BUFFER,
        .resourceInfo.buf = {
            .memType = t,
            .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            .size = size,
        },
    };

    return r->resources.size - 1;
}

u32 SEAddUniformBuffer(SEwindow *win, SERenderPipelineInfo *r, SEMemType t, u32 size) {
    dynPush(r->resources, (Resource){0});
    dynBack(r->resources) = (Resource){
        .type = RESOURCE_BUFFER,
        .resourceInfo.buf = {.memType = t, .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, .size = size},
    };

    return r->resources.size - 1;
}

VkFormat SEtoVkFormat[] = {
    [SE_IMAGE_RGBA_32] = VK_FORMAT_R32G32B32A32_SFLOAT, [SE_IMAGE_RGBA_8] = VK_FORMAT_R8G8B8A8_SRGB,
    [SE_IMAGE_RGB_32] = VK_FORMAT_R32G32B32_SFLOAT,     [SE_IMAGE_RGB_8] = VK_FORMAT_R8G8B8_SINT,
    [SE_IMAGE_BGRA_8] = VK_FORMAT_B8G8R8A8_SINT,      [SE_IMAGE_BGR_8] = VK_FORMAT_B8G8R8_SINT,
};

u32 SEAddTexture(SEwindow *win, SERenderPipelineInfo *r, SEImageFormat format, u32 width, u32 height) {
    dynPush(r->resources, (Resource){0});
    dynBack(r->resources) = (Resource) {
        .type = RESOURCE_IMAGE,
        .resourceInfo.img = {
            .usage = VK_IMAGE_USAGE_SAMPLED_BIT,
            .format = SEtoVkFormat[format],
            .width = width,
            .height = height,
            .swapRel = FALSE,
            .layout = VK_IMAGE_LAYOUT_UNDEFINED,
        },
    };

    return r->resources.size - 1;
}

void *SERetrieveDynBuf(SEwindow *win, SERenderPipeline *p, u32 buffer) {
    SEVulkan *v = GetGraphics(win);

    u32 idx = p->resourceMapping.data[buffer];

    SEBuffer *b = &p->buffers.data[idx];
    BufferAllocator *a = &p->bufAllocators.data[b->parent];
    void *out = NULL;
    vkMapMemory(v->dev, v->memory.mem.data[a->memid], b->r.offset + a->r.offset, b->r.size, 0, &out);

    return out;
}

void SEUnmapDynBuf(SEwindow *win, SERenderPipeline *p, u32 buffer) {
    SEVulkan *v = GetGraphics(win);

    u32 idx = p->resourceMapping.data[buffer];

    SEBuffer *b = &p->buffers.data[idx];
    BufferAllocator *a = &p->bufAllocators.data[b->parent];
    vkUnmapMemory(v->dev, v->memory.mem.data[a->memid]);
}

void SEBindUniformBuffer(SEwindow *win, SERenderPipeline *p, u32 pass, u32 buffer, u32 binding) {
    SEVulkan *v = GetGraphics(win);

    u32 idx = p->resourceMapping.data[buffer];

    SEBuffer *b = &p->buffers.data[idx];
    BufferAllocator *a = &p->bufAllocators.data[b->parent];

    VkDescriptorBufferInfo bufinfo = {
        .buffer = a->b,
        .offset = b->r.offset,
        .range = b->r.size,
    };

    VkWriteDescriptorSet write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .dstSet = p->passes.data[pass].pass.set,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &bufinfo,
    };

    vkUpdateDescriptorSets(v->dev, 1, &write, 0, NULL);
}

void SEBindTexture(SEwindow *win, SERenderPipeline *p, u32 pass, u32 tex, u32 sampler, u32 binding) {
    SEVulkan *v = GetGraphics(win);

    u32 idx = p->resourceMapping.data[tex];

    SEImage *b = &p->images.data[idx];

    VkDescriptorImageInfo imgInfo = {
        .imageView = b->view,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .sampler = p->samplers.data[sampler],
    };

    VkWriteDescriptorSet write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .dstSet = p->passes.data[pass].pass.set,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &imgInfo,
    };

    vkQueueWaitIdle(v->queues.transfer); //unfortunately required at the moment, maybe we should add an inflight fence for
                                         //the transfer command buffer?
    vkWaitForFences(v->dev, 1, &v->inFlight, VK_TRUE, UINT32_MAX);
    vkUpdateDescriptorSets(v->dev, 1, &write, 0, NULL);
}

void SESetScissor(SERenderPipeline *p, u32 pass, f32 x, f32 y, f32 width, f32 height) {
    Pass *passInfo = &p->passes.data[pass];
    passInfo->scissor.x = x;
    passInfo->scissor.y = y;
    passInfo->scissor.width = width;
    passInfo->scissor.height = height;
}

void SESetViewPort(SERenderPipeline *p, u32 pass, f32 x, f32 y, f32 width, f32 height) {
    Pass *passInfo = &p->passes.data[pass];
    passInfo->view = (VkViewport){
        .x = x,
        .y = y,
        .width = width,
        .height = height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
}

void SEUploadBuffer(SEwindow *win, SERenderPipeline *r, u32 resourceID, void *data, u32 size) {
    SEVulkan *v = GetGraphics(win);

    u32 idx = r->resourceMapping.data[resourceID];
    SEBuffer *b = &r->buffers.data[idx];
    BufferAllocator *a = &r->bufAllocators.data[b->parent];

    CPUtoGPUBufferMemcpy(win, a, b, data, size);
}


void SEUploadImage(SEwindow* win, SERenderPipeline* r, u32 resourceID, void* data, u32 size) {
    SEVulkan *v = GetGraphics(win);

    u32 idx = r->resourceMapping.data[resourceID];
    SEImage* img = &r->images.data[idx];

    CPUtoGPUImageMemcpy(win, img, data, img->width, img->height, 4);
}

u32 SEAddPipeline(SEwindow *win, SERenderPipelineInfo *r, u32 layout) {
    SEVulkan *v = GetGraphics(win);
    dynPush(r->pipeline, (PipelineInfo){0});
    dynBack(r->pipeline) = (PipelineInfo){
        .layout = layout,
        .attrs.a = win->mem,
        .bindings.a = win->mem,
        .pInputAssemblyState = (VkPipelineInputAssemblyStateCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .primitiveRestartEnable = FALSE,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        },
        .pTessellationState = (VkPipelineTessellationStateCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
        },
        .pViewportState = (VkPipelineViewportStateCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1,
        },
        .pRasterizationState = (VkPipelineRasterizationStateCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = FALSE,
            .rasterizerDiscardEnable = FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .lineWidth = 1.0f,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            .depthBiasEnable = FALSE,
        },
        .pMultisampleState = (VkPipelineMultisampleStateCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .sampleShadingEnable = FALSE,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        },
        .pColorBlendState = (VkPipelineColorBlendStateCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .attachmentCount = 1,
            .logicOpEnable = FALSE,
            .pAttachments = &dynBack(r->pipeline).colorBlend,
        },
        .colorBlend = (VkPipelineColorBlendAttachmentState){
            .colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            .blendEnable = FALSE,
        },
        .pDynamicState = (VkPipelineDynamicStateCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = 2,
            .pDynamicStates = dynBack(r->pipeline).dynStates,
        },
        .dynStates = {
            VK_DYNAMIC_STATE_VIEWPORT, 
            VK_DYNAMIC_STATE_SCISSOR,
        },
    };

    return r->pipeline.size - 1;
}

void SESetShaderVertex(SEwindow *win, SERenderPipelineInfo *info, u32 pipe, SString filename) {
    SEVulkan *v = GetGraphics(win);
    ScratchArena sc = ScratchArenaGet(NULL);

    file f = fileopen(filename, FILE_READ);

    char *data = ArenaAlloc(&sc.arena, f.stats.size + sizeof(u32));

    uintptr_t aligned = (uintptr_t)data;
    if (aligned % sizeof(u32))
        aligned += sizeof(u32) - (aligned % sizeof(u32));

    SString buffer = {
        .data = (i8 *)aligned,
        .len = f.stats.size,
    };
    fileread(buffer, f);
    fileclose(f);

    info->pipeline.data[pipe].vert = CompileShader(v, buffer);
    ScratchArenaEnd(sc);
}

void SESetShaderFrag(SEwindow *win, SERenderPipelineInfo *info, u32 pipe, SString filename) {
    SEVulkan *v = GetGraphics(win);
    ScratchArena sc = ScratchArenaGet(NULL);

    file f = fileopen(filename, FILE_READ);

    char *data = ArenaAlloc(&sc.arena, f.stats.size + sizeof(u32));

    uintptr_t aligned = (uintptr_t)data;
    if (aligned % sizeof(u32))
        aligned += sizeof(u32) - (aligned % sizeof(u32));

    SString buffer = {
        .data = (i8 *)aligned,
        .len = f.stats.size,
    };
    fileread(buffer, f);
    fileclose(f);

    info->pipeline.data[pipe].frag = CompileShader(v, buffer);
    ScratchArenaEnd(sc);
}

void SEAddVertexBinding(SERenderPipelineInfo *rinfo, u32 pass, SEBindingType type, SEStructSpec *layout,
                        u32 numMembers) {
    // next binding, plus increment
    PipelineInfo *p = &rinfo->pipeline.data[pass];
    u32 binding = 0; // dynBack(rinfo->passes).vertInfo.numBindings++;
    u32 size = 0;

    VkVertexInputBindingDescription desc = {
        .binding = binding,
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };

    if (type == SE_BINDING_INSTANCE)
        desc.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    for (u32 i = 0; i < numMembers; i++) {
        SEStructSpec member = layout[i];

        VkVertexInputAttributeDescription vertAttr = {
            .binding = binding,
            .location = i,
            .offset = member.offset,
        };

        switch (member.type) {
            case SE_VAR_TYPE_U32: vertAttr.format = VK_FORMAT_R32_UINT; break;
            case SE_VAR_TYPE_F32: vertAttr.format = VK_FORMAT_R32_SFLOAT; break;
            case SE_VAR_TYPE_V2F: vertAttr.format = VK_FORMAT_R32G32_SFLOAT; break;
            case SE_VAR_TYPE_V3F: vertAttr.format = VK_FORMAT_R32G32B32_SFLOAT; break;
            default: todo();
        }

        dynPush(p->attrs, vertAttr);
        size += member.size;
    }

    desc.stride = size;
    dynPush(p->bindings, desc);
}

u32 SENewPass(SEwindow *win, SERenderPipelineInfo *r) {
    dynPush(r->passes, ((PassInfo){
                           .color_attachments.a = win->mem,
                           .vertex_buffers.a = win->mem,
                       }));

    return r->passes.size - 1;
}

void SEUsePipeline(SERenderPipelineInfo *r, u32 pass, u32 pipe) { r->passes.data[pass].pipeline = pipe; }

void SEWriteColorAttachment(SEwindow *win, SERenderPipelineInfo *r, u32 pass, u32 resourceID) {
    dynPush(r->passes.data[pass].color_attachments, resourceID);
}

void SEUseVertexBuffer(SEwindow *win, SERenderPipelineInfo *r, u32 pass, u32 resourceID) {
    dynPush(r->passes.data[pass].vertex_buffers, resourceID);
}

void SESetBackBuffer(SERenderPipelineInfo *r, u32 resourceID) {
    r->backbuffer = resourceID;
    r->resources.data[resourceID].resourceInfo.img.swapRel = TRUE;
    r->resources.data[resourceID].resourceInfo.img.width = 1.0f;
    r->resources.data[resourceID].resourceInfo.img.height = 1.0f;
}

void CreateBuffer(SEwindow *win, SERenderPipelineInfo *info, SERenderPipeline *pipe, u32 resource);
void CreateImage(SEwindow *win, SERenderPipelineInfo *info, SERenderPipeline *pipe, u32 resource);
BufAllocType GetBufAlloc(SEMemType mem, VkBufferUsageFlags usage);
SEMemType GetBufMemType(BufAllocType type);
SEBufType GetBufType(BufAllocType type);

void BuildPass(SEwindow *win, SERenderPipelineInfo *info, SERenderPipeline *pipe, u32 pass);

void BuildFrameBuffers(SEVulkan *v, SERenderPipeline *pipe);
void DestroyFrameBuffers(SEVulkan *v, SERenderPipeline *pipe);

SERenderPipeline *SECompilePipeline(SEwindow *win, SERenderPipelineInfo *info) {
    SEVulkan *v = GetGraphics(win);

    SERenderPipeline *pipe = Alloc(win->mem, sizeof(SERenderPipeline));
    *pipe = (SERenderPipeline){
        .backbuffer = info->backbuffer,
        .bufAllocators.a = win->mem,
        .resourceMapping.a = win->mem,
        .images.a = win->mem,
        .imgInfos.a = win->mem,
        .buffers.a = win->mem,
        .passes.a = win->mem,
        .passVertMapping.a = win->mem,
        .framebuffers.a = win->mem,
        .framebufferInfos.a = win->mem,
        .frameBufferMapping.a = win->mem,
        .layouts.a = win->mem,
        .samplers.a = win->mem,
    };

    // init BufferAllocators
    u32 alloc_sizes[BUF_ALLOC_NUM] = {0};
    for (u32 i = 0; i < info->resources.size; i++) {
        if (info->resources.data[i].type != RESOURCE_BUFFER)
            continue;

        struct SEBufferInfo buf = info->resources.data[i].resourceInfo.buf;
        BufAllocType type = GetBufAlloc(buf.memType, buf.usage);
        assert(type != BUF_ALLOC_INVALID);
        alloc_sizes[type] += buf.size;
    }

    dynResize(pipe->bufAllocators, BUF_ALLOC_NUM);

    for (u32 i = 0; i < BUF_ALLOC_NUM; i++) {
        if (alloc_sizes[i]) {
            pipe->bufAllocators.data[i] = SEConfigBufType(win, GetBufType(i), GetBufMemType(i), alloc_sizes[i]);
        }
    }

    // build Resources
    for (u32 i = 0; i < info->resources.size; i++) {
        switch (info->resources.data[i].type) {
            case RESOURCE_BUFFER: CreateBuffer(win, info, pipe, i); break;
            case RESOURCE_IMAGE: CreateImage(win, info, pipe, i); break;
        }
    }
    pipe->backbuffer = pipe->resourceMapping.data[pipe->backbuffer];

    // Create Samplers
    dynReserve(pipe->samplers, info->samplers.size);
    for (u32 i = 0; i < info->samplers.size; i++) {
        VkSampler s;
        vkCreateSampler(v->dev, &info->samplers.data[i], NULL, &s);
        dynPush(pipe->samplers, s);
    }

    // build RenderPasses
    for (u32 i = 0; i < info->passes.size; i++) { BuildPass(win, info, pipe, i); }

    // create descriptor pools
    for (u32 i = 0; i < info->layouts.size; i++) {
        Layout l = CompileLayout(v, &info->layouts.data[i]);
        dynPush(pipe->layouts, l);
    }

    // allocate sets
    for (u32 i = 0; i < info->passes.size; i++) {
        VkDescriptorSetAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = pipe->layouts.data[pipe->passes.data[i].pass.layout].pool,
            .pSetLayouts = &info->layouts.data[pipe->passes.data[i].pass.layout].desclayout,
            .descriptorSetCount = 1,
        };
        vkAllocateDescriptorSets(v->dev, &allocInfo, &pipe->passes.data[i].pass.set);
    }

    BuildFrameBuffers(v, pipe);

    // Command Buffers
    VkCommandBufferAllocateInfo cmdInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = v->graphics.pool,
        .commandBufferCount = v->swapchain.imgcount,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    };

    pipe->cmdBufs = Alloc(win->mem, sizeof(VkCommandBuffer) * v->swapchain.imgcount);
    vkAllocateCommandBuffers(v->dev, &cmdInfo, pipe->cmdBufs);

    return pipe;
}

void SEExecutePipeline(SEwindow *win, SERenderPipeline *p) {
    SEVulkan *v = GetGraphics(win);

    static u32 counter = 0;
    static u32 prev = 0;
    prev = counter;
    counter = (counter + 1) % v->swapchain.imgcount;

    vkWaitForFences(v->dev, 1, &v->inFlight, TRUE, UINT32_MAX);

    vkResetCommandBuffer(p->cmdBufs[counter], 0);

    u32 idx = 0;
    VkResult result;
    result = vkAcquireNextImageKHR(v->dev, v->swapchain.swap, UINT32_MAX, v->imgAvalible.data[counter], NULL, &idx);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        vkDeviceWaitIdle(v->dev);
        CreateSwapChain(win, v, win->mem);
        DestroyFrameBuffers(v, p);

        for (u32 i = 0; i < p->imgInfos.size; i++) {
            struct SEImageInfo *info = &p->imgInfos.data[i];
            if (!info->swapRel)
                continue;
            SEImage *img = &p->images.data[i];
            FreeDeviceMem(&v->memory.heaps.data[img->memid], img->r);
            vkDestroyImageView(v->dev, img->view, NULL);
            vkDestroyImage(v->dev, img->img, NULL);

            p->images.data[i] = AllocImage(v, info->usage, info->format, v->swapchain.width * info->width,
                                           v->swapchain.height * info->height);
        }

        BuildFrameBuffers(v, p);
        // return;
    }

    // reset after possible resize
    vkResetFences(v->dev, 1, &v->inFlight);

    VkCommandBufferBeginInfo cmdBeg = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };
    vkBeginCommandBuffer(p->cmdBufs[counter], &cmdBeg);

    for (u32 i = 0; i < p->passes.size; i++) {
        Pass *pass = &p->passes.data[i];

        VkRenderPassBeginInfo rbeg = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = pass->pass.pass,
            .framebuffer = p->framebuffers.data[pass->pass.framebuffer],
            .renderArea = {
                .offset = {0, 0},
                .extent = {pass->size.x, pass->size.y},
            },
            .clearValueCount = 1,
            .pClearValues = &(VkClearValue){.color.float32 = {0, 0, 0}},
        };

        vkCmdBeginRenderPass(p->cmdBufs[counter], &rbeg, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(p->cmdBufs[counter], VK_PIPELINE_BIND_POINT_GRAPHICS, pass->pass.pipeline);

        VkViewport view = pass->view;
        view.x *= pass->size.x;
        view.x *= pass->size.y;
        view.width *= pass->size.x;
        view.height *= pass->size.y;
        vkCmdSetViewport(p->cmdBufs[counter], 0, 1, &view);

        VkRect2D scissor = (VkRect2D){
            .offset.x = pass->scissor.x * pass->size.x,
            .offset.y = pass->scissor.y * pass->size.y,
            .extent.width = pass->scissor.width * pass->size.x,
            .extent.height = pass->scissor.height * pass->size.y,
        };
        vkCmdSetScissor(p->cmdBufs[counter], 0, 1, &scissor);

        ScratchArena sc = ScratchArenaGet(NULL);
        if (pass->resources.verts.num) {
            VkBuffer *vertBuffers = ArenaAlloc(&sc.arena, sizeof(VkBuffer) * pass->resources.verts.num);
            VkDeviceSize *vertOffsets = ArenaAlloc(&sc.arena, sizeof(VkDeviceSize) * pass->resources.verts.num);

            for (u32 i = 0; i < pass->resources.verts.num; i++) {
                u32 memid = p->buffers.data[pass->resources.verts.start].parent;
                vertBuffers[i] = p->bufAllocators.data[memid].b;
                vertOffsets[i] = p->buffers.data[pass->resources.verts.start].r.offset;
            }
            vkCmdBindVertexBuffers(p->cmdBufs[counter], 0, pass->resources.verts.num, vertBuffers, vertOffsets);
        }
        ScratchArenaEnd(sc);

        vkCmdBindDescriptorSets(p->cmdBufs[counter], VK_PIPELINE_BIND_POINT_GRAPHICS, pass->pass.pipeLayout, 0, 1,
                                &pass->pass.set, 0, NULL);

        // pass->draw();
        vkCmdDraw(p->cmdBufs[counter], 3, 1, 0, 0);

        vkCmdEndRenderPass(p->cmdBufs[counter]);
    }

    SEImage *src = &p->images.data[p->backbuffer];

    vkCmdPipelineBarrier(p->cmdBufs[counter],
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0,
            NULL,
            0,
            NULL,
            2,
            (VkImageMemoryBarrier[]){ 
                (VkImageMemoryBarrier){
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image = src->img,
                    .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                    .dstAccessMask = 0,
                    .subresourceRange = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseArrayLayer = 0,
                        .baseMipLevel = 0,
                        .layerCount = 1,
                        .levelCount = 1,
                    },
                },
                (VkImageMemoryBarrier){
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image = v->swapchain.imgs[idx],
                    .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                    .dstAccessMask = 0,
                    .subresourceRange = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseArrayLayer = 0,
                        .baseMipLevel = 0,
                        .layerCount = 1,
                        .levelCount = 1,
                    },
                },
            });

    VkImageBlit b = {
        .srcOffsets = {
            {0, 0, 0},
            {src->width, src->height, 1},
        },
        .srcSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseArrayLayer = 0,
            .layerCount = 1,
            .mipLevel = 0,
        },
        .dstOffsets = {
            {0, 0, 0},
            {v->swapchain.width, v->swapchain.height, 1},
        },
        .dstSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseArrayLayer = 0,
            .layerCount = 1,
            .mipLevel = 0,
        },
    };

    vkCmdBlitImage(p->cmdBufs[counter], src->img, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, v->swapchain.imgs[idx],
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &b, VK_FILTER_NEAREST);

    vkCmdPipelineBarrier(p->cmdBufs[counter],
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            0,
            0,
            NULL,
            0,
            NULL,
            1,
            &(VkImageMemoryBarrier){
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = v->swapchain.imgs[idx],
                .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                .dstAccessMask = 0,
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseArrayLayer = 0,
                    .baseMipLevel = 0,
                    .layerCount = 1,
                    .levelCount = 1,
                },
            });

    vkEndCommandBuffer(p->cmdBufs[counter]);

    VkPipelineStageFlagBits stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo subInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &p->cmdBufs[counter],
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &v->imgAvalible.data[counter],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &v->renderfinished.data[idx],
        .pWaitDstStageMask = &stages,
    };

    vkQueueSubmit(v->queues.graphics, 1, &subInfo, v->inFlight);

    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &v->renderfinished.data[idx],
        .swapchainCount = 1,
        .pSwapchains = &v->swapchain.swap,
        .pImageIndices = &idx,
    };

    result = vkQueuePresentKHR(v->queues.present, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || win->resize) {
        vkDeviceWaitIdle(v->dev);
        CreateSwapChain(win, v, win->mem);
        DestroyFrameBuffers(v, p);

        for (u32 i = 0; i < p->imgInfos.size; i++) {
            struct SEImageInfo *info = &p->imgInfos.data[i];
            if (!info->swapRel)
                continue;
            SEImage *img = &p->images.data[i];
            FreeDeviceMem(&v->memory.heaps.data[img->memid], img->r);
            vkDestroyImageView(v->dev, img->view, NULL);
            vkDestroyImage(v->dev, img->img, NULL);

            p->images.data[i] = AllocImage(v, info->usage, info->format, v->swapchain.width * info->width,
                                           v->swapchain.height * info->height);
        }

        BuildFrameBuffers(v, p);
        win->resize = FALSE;
        return;
    }
}

void BuildPass(SEwindow *win, SERenderPipelineInfo *info, SERenderPipeline *pipe, u32 pidx) {
    SEVulkan *v = GetGraphics(win);
    PassInfo *pass = &info->passes.data[pidx];

    ScratchArena sc = ScratchArenaGet(NULL);

    u32 num_attachments = pass->color_attachments.size;
    VkAttachmentReference *color_attachments
        = ArenaAlloc(&sc.arena, sizeof(VkAttachmentReference) * pass->color_attachments.size);
    VkAttachmentDescription *descrips = ArenaAllocZero(&sc.arena, sizeof(VkAttachmentReference) * num_attachments);

    FrameBufferInfo framebuf = {
        .first = pipe->frameBufferMapping.size,
        .num = num_attachments,
    };

    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .pColorAttachments = color_attachments,
        .colorAttachmentCount = pass->color_attachments.size,
    };

    VkSubpassDependency dep = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    };

    if (pass->color_attachments.size) {
        dep.dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dep.dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dep.srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }

    for (u32 i = 0; i < pass->color_attachments.size; i++) {

        u32 imgIdx = pipe->resourceMapping.data[pass->color_attachments.data[i]];
        dynPush(pipe->frameBufferMapping, imgIdx);

        struct SEImageInfo *res = &info->resources.data[pass->color_attachments.data[i]].resourceInfo.img;

        descrips[i].format = res->format;
        descrips[i].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        descrips[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        descrips[i].samples = VK_SAMPLE_COUNT_1_BIT;
        descrips[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        descrips[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        switch (res->lastAccess) {
            case ACCESS_UNINITIALIZED:
                descrips[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                descrips[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

                dep.srcAccessMask |= 0;
                break;
            case ACCESS_PREINITIALIZED:
                descrips[i].initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
                descrips[i].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

                dep.srcAccessMask |= 0;
                break;
            case ACCESS_COLOR_ATTACHMENT:
                descrips[i].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                descrips[i].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

                dep.srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;
        }

        if (!res->persist) {
            descrips[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        }

        res->lastAccess = ACCESS_COLOR_ATTACHMENT;

        color_attachments[i].attachment = i;
        color_attachments[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    VkRenderPassCreateInfo rpass = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pAttachments = descrips,
        .attachmentCount = num_attachments,
        .pSubpasses = &subpass,
        .subpassCount = 1,
        .pDependencies = &dep,
        .dependencyCount = 1,
    };

    Pass final = {
        .pass.framebuffer = pipe->framebufferInfos.size,
        .resources.verts = {
            .start = pipe->passVertMapping.size,
            .num = pass->vertex_buffers.size,
        },
    };

    dynPush(pipe->framebufferInfos, framebuf);

    for (u32 i = 0; i < pass->vertex_buffers.size; i++) {
        u32 bufid = pipe->resourceMapping.data[pass->vertex_buffers.data[i]];
        dynPush(pipe->passVertMapping, bufid);
    }

    vkCreateRenderPass(v->dev, &rpass, NULL, &final.pass.pass);
    final.pass.layout = info->pipeline.data[pass->pipeline].layout;
    final.pass.pipeLayout = CreatePipelineLayout(v, &info->layouts.data[final.pass.layout]);
    final.pass.pipeline
        = CreatePipeline(v, final.pass.pass, &info->pipeline.data[pass->pipeline], final.pass.pipeLayout);
    info->layouts.data[info->pipeline.data[pass->pipeline].layout].sets_required++;

    dynPush(pipe->passes, final);
    ScratchArenaEnd(sc);
}

void BuildFrameBuffers(SEVulkan *v, SERenderPipeline *pipe) {

    ScratchArena sc = ScratchArenaGet(NULL);

    for (u32 i = 0; i < pipe->passes.size; i++) {
        FrameBufferInfo *frame = &pipe->framebufferInfos.data[pipe->passes.data[i].pass.framebuffer];
        VkImageView *views = ArenaAlloc(&sc.arena, sizeof(VkImageView) * frame->num);

        v2u size = {UINT32_MAX, UINT32_MAX};
        for (u32 j = 0; j < frame->num; j++) {
            views[j] = pipe->images.data[j + frame->first].view;
            SEImage *img = &pipe->images.data[j + frame->first];
            size.x = MIN(size.x, img->width);
            size.y = MIN(size.y, img->height);
        }

        pipe->passes.data[i].size = size;
        debuglog("frameBuffer Size = (%d, %d)", size.x, size.y);

        VkFramebufferCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pAttachments = views,
            .attachmentCount = frame->num,
            .width = size.x,
            .height = size.y,
            .layers = 1,
            .renderPass = pipe->passes.data[i].pass.pass,
        };

        VkFramebuffer buf;
        vkCreateFramebuffer(v->dev, &info, NULL, &buf);
        dynPush(pipe->framebuffers, buf);

        ArenaPop(&sc.arena, sizeof(VkImageView) * frame->num);
    }

    ScratchArenaEnd(sc);
}

void DestroyFrameBuffers(SEVulkan *v, SERenderPipeline *pipe) {
    for (u32 i = 0; i < pipe->framebuffers.size; i++) {
        vkDestroyFramebuffer(v->dev, pipe->framebuffers.data[i], NULL);
    }
    pipe->framebuffers.size = 0;
}

void CreateBuffer(SEwindow *win, SERenderPipelineInfo *info, SERenderPipeline *pipe, u32 resource) {
    struct SEBufferInfo bufInfo = info->resources.data[resource].resourceInfo.buf;

    BufAllocType t = GetBufAlloc(bufInfo.memType, bufInfo.usage);
    SEBuffer buffer = AllocBuffer(win, &pipe->bufAllocators.data[t], t, bufInfo.size);

    dynPush(pipe->resourceMapping, pipe->buffers.size);
    dynPush(pipe->buffers, buffer);
}

void CreateImage(SEwindow *win, SERenderPipelineInfo *info, SERenderPipeline *pipe, u32 resource) {
    SEVulkan *v = GetGraphics(win);
    struct SEImageInfo imgInfo = info->resources.data[resource].resourceInfo.img;

    u32 width, height;
    if (imgInfo.swapRel) {
        width = v->swapchain.width * imgInfo.width;
        height = v->swapchain.height * imgInfo.height;
    } else {
        width = imgInfo.width;
        height = imgInfo.height;
    }

    SEImage img = AllocImage(v, imgInfo.usage, imgInfo.format, width, height);

    dynPush(pipe->resourceMapping, pipe->images.size);
    dynPush(pipe->images, img);
    dynPush(pipe->imgInfos, imgInfo);
}

BufAllocType GetBufAlloc(SEMemType mem, VkBufferUsageFlags usage) {
    if (mem == SE_MEM_STATIC) {
        if (usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
            return BUF_ALLOC_VERT_STATIC;

        if (usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
            return BUF_ALLOC_UNIFORM_STATIC;
    } else {
        if (usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
            return BUF_ALLOC_VERT_DYN;

        if (usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
            return BUF_ALLOC_UNIFORM_DYN;
    }

    return BUF_ALLOC_INVALID;
}

SEMemType GetBufMemType(BufAllocType type) {
    switch (type) {
        case BUF_ALLOC_VERT_STATIC:
        case BUF_ALLOC_UNIFORM_STATIC: return SE_MEM_STATIC;

        case BUF_ALLOC_VERT_DYN:
        case BUF_ALLOC_UNIFORM_DYN: return SE_MEM_DYNAMIC;

        default: {
            debugerr("Invalid Buf Type");
            panic();
        }
    }
}

SEBufType GetBufType(BufAllocType type) {
    switch (type) {
        case BUF_ALLOC_VERT_DYN:
        case BUF_ALLOC_VERT_STATIC: return SE_BUFFER_VERT;
        case BUF_ALLOC_UNIFORM_STATIC:
        case BUF_ALLOC_UNIFORM_DYN: return SE_BUFFER_UNIFORM;

        default: {
            debugerr("Invalid Type");
            panic();
        }
    }
}

void SEDestroyRenderPipelineInfo(SEwindow *win, SERenderPipelineInfo *r) {
    SEVulkan *v = GetGraphics(win);
    vkDeviceWaitIdle(v->dev);
    dynFree(r->resources);
    dynFree(r->samplers);

    for (u32 i = 0; i < r->passes.size; i++) {
        dynFree(r->passes.data[i].color_attachments);
        dynFree(r->passes.data[i].vertex_buffers);
    }
    dynFree(r->passes);

    for (u32 i = 0; i < r->passes.size; i++) {
        dynFree(r->pipeline.data[i].bindings);
        dynFree(r->pipeline.data[i].attrs);

        vkDestroyShaderModule(v->dev, r->pipeline.data[i].vert, NULL);
        vkDestroyShaderModule(v->dev, r->pipeline.data[i].frag, NULL);

        // vkDestroyPipelineLayout(v->dev, r->pipeline.data[i].layout, NULL);
    }
    dynFree(r->pipeline);

    for (u32 i = 0; i < r->layouts.size; i++) {
        dynFree(r->layouts.data[i].bindings);

        vkDestroyPipelineLayout(v->dev, r->layouts.data[i].layout, NULL);
        vkDestroyDescriptorSetLayout(v->dev, r->layouts.data[i].desclayout, NULL);
    }
    dynFree(r->layouts);
}

void SEDestroyPipeline(SEwindow *win, SERenderPipeline *p) {
    SEVulkan *v = GetGraphics(win);
    vkDeviceWaitIdle(v->dev);

    Free(win->mem, p->cmdBufs, v->swapchain.imgcount);

    dynFree(p->resourceMapping);
    dynFree(p->passVertMapping);
    dynFree(p->frameBufferMapping);
    dynFree(p->framebufferInfos);
    dynFree(p->imgInfos);
    dynFree(p->buffers);

    for (u32 i = 0; i < p->samplers.size; i++) {
        vkDestroySampler(v->dev, p->samplers.data[i], NULL);
    }
    dynFree(p->samplers);

    for (u32 i = 0; i < p->passes.size; i++) {
        vkDestroyPipeline(v->dev, p->passes.data[i].pass.pipeline, NULL);
        vkDestroyRenderPass(v->dev, p->passes.data[i].pass.pass, NULL);
        vkDestroyPipelineLayout(v->dev, p->passes.data[i].pass.pipeLayout, NULL);
    }
    dynFree(p->passes);

    for (u32 i = 0; i < p->layouts.size; i++) { vkDestroyDescriptorPool(v->dev, p->layouts.data[i].pool, NULL); }
    dynFree(p->layouts);

    for (u32 i = 0; i < p->bufAllocators.size; i++) {
        BufferAllocator *a = &p->bufAllocators.data[i];
        if (!a->r.size)
            continue;
        FreeDeviceMem(&v->memory.heaps.data[a->memid], a->r);
        DestroyManager(a->m);
        vkDestroyBuffer(v->dev, a->b, NULL);
    }
    dynFree(p->bufAllocators);

    DestroyFrameBuffers(v, p);
    dynFree(p->framebuffers);

    for (u32 i = 0; i < p->images.size; i++) {
        SEImage *img = &p->images.data[i];
        FreeDeviceMem(&v->memory.heaps.data[img->memid], img->r);
        vkDestroyImageView(v->dev, img->view, NULL);
        vkDestroyImage(v->dev, img->img, NULL);
    }
    dynFree(p->images);
}
