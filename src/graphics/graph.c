#include "cutils.h"
#include "ds.h"
#include "se.h"
#include "vulkan/vulkan_core.h"
#include <graphics/graphics_intern.h>

//INFO(ELI):
/*

    Basically the plan is to create an attachment
    and mark it as the backbuffer.

    Then at the end we do a vkCmdImgBlit to copy
    the backbuffer to the swapchain.

*/

SERenderPipelineInfo *SECreatePipeline(SEwindow *win) {
    SEVulkan *v = GetGraphics(win);

    SERenderPipelineInfo *r = Alloc(win->mem, sizeof(SERenderPipelineInfo));
    *r = (SERenderPipelineInfo){
        .a = win->mem,
        .passes.a = win->mem,
        .resources.a = win->mem,
        .reads.a = win->mem,
        .writes.a = win->mem,
        .shaders.a = win->mem,
        .layouts.a = win->mem,
        .attrs.a = win->mem,
        .bindings.a = win->mem,
        .vertBufInfo.a = win->mem,
        .indexBufInfo.a = win->mem,
    };

    return r;
}

u32 SEAddShader(SEwindow *win, SERenderPipelineInfo *r, SString source) {
    SEVulkan *g = GetGraphics(win);

    file src = fileopen(source, FILE_READ);

    SString code = {.len = src.stats.size, .data = Alloc(win->mem, src.stats.size)};

    u64 read = fileread(code, src);
    fileclose(src);

    VkShaderModuleCreateInfo shaderInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = read,
        .pCode = (u32 *)code.data,
    };

    u32 new = ShaderPoolAlloc(&g->resources.shaders);
    VkShaderModule m;
    vkCreateShaderModule(g->dev, &shaderInfo, NULL, &m);
    Free(win->mem, code.data, code.len);

    dynPush(r->shaders, m);

    return r->shaders.size - 1;
}

u32 SEAddLayout(SEwindow *win, SERenderPipelineInfo *r) {
    SEVulkan *v = GetGraphics(win);
    VkPipelineLayout layout;
    VkPipelineLayoutCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    };
    vkCreatePipelineLayout(v->dev, &info, NULL, &layout);
    dynPush(r->layouts, layout);

    return r->layouts.size - 1;
}

u32 SEAddVertexBuffer(SERenderPipelineInfo* r, SEMemType type, u32 size) {
    dynPush(r->resources, ((Resource){ 
            .type = RESOURCE_BUFFER,
            .lastUsage = RESOURCE_READ,
            .vk.buf = {
                .usage = BUFFER_USAGE_VERTEX,
                .idx = r->vertBufInfo.size,
            }
        }));

    dynPush(r->vertBufInfo, ((BufferInfo){
                .memType = type,
                .size = size,
        }));

    return r->resources.size - 1;
}

u32 SEAddColorAttachment(SERenderPipelineInfo* r, u32 width, u32 height) {
    dynPush(r->resources, ((Resource){ 
            .type = RESOURCE_IMAGE,
            .lastUsage = RESOURCE_UNINITIALIZED,
        }));

    return r->resources.size - 1;
}

u32 SEAddImg(SERenderPipelineInfo *r, u32 width, u32 height, bool8 clear) {
    dynPush(r->resources, ((Resource){.lastUsage = RESOURCE_READ, .type = RESOURCE_IMAGE, 
                .vk.img = {
                    .width = width,
                    .height = height,
                    .clear = clear,
                }}));
    return r->resources.size - 1;
}

void SESetBackBuffer(SERenderPipelineInfo* r, u32 resourceID) {
    r->backbuffer = resourceID;
}

void SEBindShaders(SERenderPipelineInfo *r, u32 vert, u32 frag, u32 layout) {
    dynBack(r->passes).pipeline.vertex = vert;
    dynBack(r->passes).pipeline.fragment = frag;
    dynBack(r->passes).pipeline.layout = layout;
}

void SEReadResource(SERenderPipelineInfo *r, u32 resourceID) {
    dynPush(r->reads, resourceID);
    dynBack(r->passes).numReads++;
}

void SEUseVertexBuffer(SERenderPipelineInfo* r, u32 resourceID) {
    dynPush(r->reads, resourceID);
}

void SEUseColorAttachment(SERenderPipelineInfo* r, u32 resourceID) {
    dynPush(r->writes, resourceID);
    dynPush(r->writeInfo, WRITE_IMG_COLOR_ATTACHMENT);
    r->resources.data[resourceID].vk.img.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
}

void SEWriteResource(SERenderPipelineInfo *r, u32 resourceID) {
    dynPush(r->writes, resourceID);
    dynBack(r->passes).numWrites++;
}

void DrawTriangle(SECmdBuf *p, void *pass) { vkCmdDraw(p->buf, 3, 1, 0, 0); }

void SEBeginRenderPass(SERenderPipelineInfo *r) {
    dynPush(r->passes, ((SERenderPassInfo){
                           .readStart = r->reads.size,
                           .writeStart = r->writes.size,
                           .vertInfo.firstAttr = r->attrs.size,
                           .vertInfo.firstBinding = r->bindings.size,
                       }));
}

void SEEndRenderPass(SERenderPipelineInfo *r) {}

void SEAddDrawFunc(SERenderPipelineInfo *r, SEDrawFunc f) { dynBack(r->passes).func = f; }

typedef struct PipelineBuildInfo {
    SERenderPipeline* p;
    dynArray(VkAttachmentDescription) descrips;
    dynArray(VkAttachmentReference) refs;
    dynArray(VkSubpassDependency) deps;
    dynArray(VkImageView) views;
    //dynArray(SEBuffer) vertexBuffers;
    u32 vert_size;
    u32 curr_vert_size;
    bool8 pushBarrier;
    PipelineBarrier barrier;
} PipelineBuildInfo;

void ReadImage(PipelineBuildInfo* buildInfo, ReadType t, Resource* obj) {
        SEImage img = buildInfo->p->images.data[obj->vk.img.idx];
        dynPush(buildInfo->views, img.view);

        VkAttachmentDescription descrip = {};

        VkAttachmentReference ref = {
            .attachment = buildInfo->descrips.size,
        };

        VkSubpassDependency dep = {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
        };

        descrip.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        descrip.format = obj->vk.img.format;

        switch (t) {
            case READ_IMG_INPUT_ATTACHMENT:
            {
                ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            }
            break;
            default: panic();
        }

        switch (obj->lastUsage) {
            case RESOURCE_UNINITIALIZED: {
                panic(); // should never happen
            } break;
            case RESOURCE_READ: {
                descrip.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                descrip.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                descrip.samples = VK_SAMPLE_COUNT_1_BIT;

                descrip.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

            } break;
            case RESOURCE_WRITE: {
                descrip.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                descrip.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                descrip.samples = VK_SAMPLE_COUNT_1_BIT;

                descrip.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

                dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dep.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

                dep.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

                // NOTE(ELI): For now just block everywhere the attachment could be used
                // namely input attachment, shader attachment, or color read.
                dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_SHADER_READ_BIT;

                dynPush(buildInfo->deps, dep);
            } break;
        }
        obj->lastUsage = RESOURCE_READ;
        dynPush(buildInfo->descrips, descrip);
        dynPush(buildInfo->refs, ref);
}

bool8 WriteImage(PipelineBuildInfo* buildInfo, WriteType t, Resource* obj) {
        bool8 swapchain = FALSE;
        if (obj->swapchain)
            swapchain = TRUE;
        else {
            SEImage img = buildInfo->p->images.data[obj->vk.img.idx];
            dynPush(buildInfo->views, img.view);
        }

        VkAttachmentDescription descrip = {};

        VkAttachmentReference ref = {
            .attachment = buildInfo->descrips.size,
        };

        VkSubpassDependency dep = {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
        };

        descrip.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        descrip.format = obj->vk.img.format;

        switch (t) {
            case WRITE_IMG_COLOR_ATTACHMENT:
            {
                ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            }
            break;
            default: panic();
        }

        switch (obj->lastUsage) {
            case RESOURCE_UNINITIALIZED: {

                descrip.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                descrip.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                descrip.samples = VK_SAMPLE_COUNT_1_BIT;

                if (obj->clear)
                    descrip.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                else
                    descrip.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;


                dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

                dynPush(buildInfo->deps, dep);
            } break;
            case RESOURCE_READ: {
                debuglog("hit");

                descrip.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                descrip.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                descrip.samples = VK_SAMPLE_COUNT_1_BIT;

                descrip.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

                dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

                dep.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT |
                                    VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;

                dynPush(buildInfo->deps, dep);
            } break;
            case RESOURCE_WRITE: {
                debuglog("hit");
                descrip.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                descrip.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                descrip.samples = VK_SAMPLE_COUNT_1_BIT;

                descrip.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;


                dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

                dep.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                dynPush(buildInfo->deps, dep);
            } break;
        }

        obj->lastUsage = RESOURCE_WRITE;

        dynPush(buildInfo->descrips, descrip);
        dynPush(buildInfo->refs, ref);
        return swapchain;
}

void ReadBuffer(PipelineBuildInfo* buildInfo, Resource* obj) {
    switch (obj->lastUsage) {
        case RESOURCE_UNINITIALIZED:
            todo();
            break;
        case RESOURCE_READ:
            break;
        case RESOURCE_WRITE:
            break;
    }

    //dynPush(buildInfo->vertexBuffers, obj->vk.buffer);
    buildInfo->curr_vert_size++;
}

void BuildPass(PipelineBuildInfo *buildInfo, SEVulkan *v, SERenderPipelineInfo *r, SERenderPipeline *p,
               SERenderPassInfo *passInfo) {

    //Process Reads and Writes
    bool8 swapchain = FALSE;
    {
        buildInfo->pushBarrier = FALSE;
        buildInfo->barrier = (PipelineBarrier){0};

        u32 offset = passInfo->readStart;
        for (u32 j = 0; j < passInfo->numReads; j++) {
            Resource *obj = &r->resources.data[r->reads.data[offset + j]];
            switch (obj->type) {
                case RESOURCE_IMAGE: ReadImage(buildInfo, r->readInfo.data[offset + j], obj); break;
                case RESOURCE_BUFFER: ReadBuffer(buildInfo, obj); break;
                default: todo();
            }
            debuglog("\tRead: %d", r->reads.data[j]);
        }

        offset = passInfo->writeStart;
        for (u32 j = 0; j < passInfo->numWrites; j++) {
            Resource *obj = &r->resources.data[r->writes.data[offset + j]];

            switch (obj->type) {
                case RESOURCE_IMAGE: swapchain = WriteImage(buildInfo, r->writeInfo.data[offset + j], obj); break;
                default: todo();
            }

            debuglog("\tWrite: %d", r->writes.data[j]);
        }

        if (buildInfo->pushBarrier)
            dynPush(p->barriers, buildInfo->barrier);
    }

    VkSubpassDescription subdescrip = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = buildInfo->refs.size,
        .pColorAttachments = buildInfo->refs.data,
    };

    VkRenderPassCreateInfo renderInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = buildInfo->descrips.size,
        .pAttachments = buildInfo->descrips.data,
        .dependencyCount = buildInfo->deps.size,
        .pDependencies = buildInfo->deps.data,
        .subpassCount = 1,
        .pSubpasses = &subdescrip,
    };

    SEPass pass = {
        .framebuffer = p->framebuffers.size,
        .targetSwap = swapchain,
        .func = (SEDrawFunc)passInfo->func,
        .vertbufs = { 
            .start = buildInfo->vert_size,
            .size = buildInfo->curr_vert_size, 
        },
    };
    debuglog("Pass Vert: %d %d", pass.vertbufs.start, pass.vertbufs.size);
    debuglog("targetSwap: %d", swapchain);

    buildInfo->vert_size += buildInfo->curr_vert_size;

    vkCreateRenderPass(v->dev, &renderInfo, NULL, &pass.rpass);
    pass.pipe =
        CreatePipeline(v, pass.rpass, &r->bindings.data[passInfo->vertInfo.firstAttr], passInfo->vertInfo.numBindings,
                       &r->attrs.data[passInfo->vertInfo.firstBinding], passInfo->vertInfo.numAttrs,
                       r->layouts.data[passInfo->pipeline.layout], r->shaders.data[passInfo->pipeline.vertex],
                       r->shaders.data[passInfo->pipeline.fragment]);

    if (!swapchain) {
        VkFramebufferCreateInfo frame = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .height = v->swapchain.height,
            .width = v->swapchain.width,
            .layers = 1,
            .renderPass = pass.rpass,
            .attachmentCount = buildInfo->views.size,
            .pAttachments = buildInfo->views.data,
        };

        VkFramebuffer buf;
        vkCreateFramebuffer(v->dev, &frame, NULL, &buf);
        dynPush(p->framebuffers, buf);
    } else {
        //dynPush(buildInfo->views, r->resources.data[0].vk.view);
        for (u32 i = 0; i < v->swapchain.imgcount; i++) {
            dynBack(buildInfo->views) = v->swapchain.views[i];
            VkFramebufferCreateInfo frame = {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .height = v->swapchain.height,
                .width = v->swapchain.width,
                .layers = 1,
                .renderPass = pass.rpass,
                .attachmentCount = buildInfo->views.size,
                .pAttachments = buildInfo->views.data,
            };

            VkFramebuffer buf;
            vkCreateFramebuffer(v->dev, &frame, NULL, &buf);
            dynPush(p->framebuffers, buf);
        }
    }

    dynPush(p->passes, pass);

    dynResize(buildInfo->deps, 0);
    dynResize(buildInfo->descrips, 0);
    dynResize(buildInfo->refs, 0);
    dynResize(buildInfo->views, 0);
}

SERenderPipeline *SECompilePipeline(SEwindow *win, SERenderPipelineInfo *r) {
    SEVulkan *v = GetGraphics(win);

    SERenderPipeline *p = Alloc(win->mem, sizeof(SERenderPipeline));

    *p = (SERenderPipeline){
        .a = win->mem,
        .barriers.a = win->mem,
        .passes.a = win->mem,
        .vertexBuffers.a = win->mem,
        .indexBuffers.a = win->mem,
        .framebuffers.a = win->mem,
        .buf.a = win->mem,
        .bufAllocators.a = win->mem,
        .resourceMaps.a = win->mem,
        .images.a = win->mem,
    };

    PipelineBuildInfo buildInfo = {
        .p = p,
        .descrips.a = win->mem,
        .refs.a = win->mem,
        .deps.a = win->mem,
        .views.a = win->mem,
    };



    ScratchArena sc = ScratchArenaGet(NULL);
    VkFormat* formats = ArenaAlloc(&sc.arena, sizeof(VkFormat) * 50);
    //Create Attachments
    for (u32 i = 0; i < r->resources.size; i++) {
        if (r->resources.data[i].type != RESOURCE_IMAGE) continue;
        u32 formatnum = 0;
        
        if (r->resources.data[i].vk.img.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
            formats[formatnum++] = v->swapchain.format.format;
        }

        if (r->resources.data[i].vk.img.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            formats[formatnum++] = VK_FORMAT_D32_SFLOAT;
            formats[formatnum++] = VK_FORMAT_D32_SFLOAT_S8_UINT;
            formats[formatnum++] = VK_FORMAT_D24_UNORM_S8_UINT;
        }

        r->resources.data[i].vk.img.idx = p->images.size;
        dynPush(p->images, AllocImage(v, r->resources.data[i].vk.img.usage,
                    formats, formatnum,
                    r->resources.data[i].vk.img.width, r->resources.data[i].vk.img.height));
    
    }
    ScratchArenaEnd(sc);
    p->backbuffer = r->backbuffer;

    // INFO(ELI):
    // The subpass dependencies take care of virtually all
    // operations which do not cross queue boundaries
    //
    // Barriers are only required when moving between
    // queues or when doing work outside of a renderpass

    for (u32 i = 0; i < r->passes.size; i++) { BuildPass(&buildInfo, v, r, p, &r->passes.data[i]); }

    dynFree(buildInfo.deps);
    dynFree(buildInfo.descrips);
    dynFree(buildInfo.refs);
    dynFree(buildInfo.views);

    // allocate command buffer
    VkCommandBufferAllocateInfo bufinfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandBufferCount = v->swapchain.imgcount,
        .commandPool = v->pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    };

    dynResize(p->buf, v->swapchain.imgcount);

    vkAllocateCommandBuffers(v->dev, &bufinfo, p->buf.data);


    //allocate buffers

    u32 static_vert_size = 0;
    u32 static_indx_size = 0;
    u32 dyn_vert_size = 0;
    u32 dyn_indx_size = 0;
    for (u32 i = 0; i < r->vertBufInfo.size; i++) {
        BufferInfo b = r->vertBufInfo.data[i];
        switch (b.memType) {
            case SE_MEM_STATIC: 
                static_vert_size += b.size;
            break;
            case SE_MEM_DYNAMIC: 
                dyn_vert_size += b.size;
            break;
        }
    }

    for (u32 i = 0; i < r->indexBufInfo.size; i++) {
        BufferInfo b = r->indexBufInfo.data[i];
        switch (b.memType) {
            case SE_MEM_STATIC: 
                static_indx_size += b.size;
            break;
            case SE_MEM_DYNAMIC: 
                dyn_indx_size += b.size;
            break;
        }
    }

    u32 static_vert = p->bufAllocators.size;
    if (static_vert_size)
        dynPush(p->bufAllocators, SEConfigBufType(win, SE_BUFFER_VERT, SE_MEM_STATIC, static_vert_size));
    u32 static_index = p->bufAllocators.size;
    if (static_indx_size)
        dynPush(p->bufAllocators, SEConfigBufType(win, SE_BUFFER_INDEX, SE_MEM_STATIC, static_indx_size));

    u32 dyn_vert = p->bufAllocators.size;
    if (dyn_vert_size)
        dynPush(p->bufAllocators, SEConfigBufType(win, SE_BUFFER_VERT, SE_MEM_DYNAMIC, dyn_vert_size));
    u32 dyn_index = p->bufAllocators.size;
    if (dyn_indx_size)
        dynPush(p->bufAllocators, SEConfigBufType(win, SE_BUFFER_INDEX, SE_MEM_DYNAMIC, dyn_indx_size));

    for (u32 i = 0; i < r->vertBufInfo.size; i++) {
        BufferInfo b = r->vertBufInfo.data[i];
        SEBuffer buf;
        switch (b.memType) {
            case SE_MEM_STATIC: 
                buf = AllocBuffer(win, &p->bufAllocators.data[static_vert], static_vert, b.size);
                dynPush(p->vertexBuffers, buf);
            break;
            case SE_MEM_DYNAMIC: 
                buf = AllocBuffer(win, &p->bufAllocators.data[dyn_vert], dyn_vert, b.size);
                dynPush(p->vertexBuffers, buf);
            break;
        }
    }

    for (u32 i = 0; i < r->indexBufInfo.size; i++) {
        BufferInfo b = r->indexBufInfo.data[i];
        SEBuffer buf;
        switch (b.memType) {
            case SE_MEM_STATIC: 
                buf = AllocBuffer(win, &p->bufAllocators.data[static_index], static_index, b.size);
                dynPush(p->indexBuffers, buf);
            break;
            case SE_MEM_DYNAMIC: 
                buf = AllocBuffer(win, &p->bufAllocators.data[dyn_index], dyn_index, b.size);
                dynPush(p->indexBuffers, buf);
            break;
        }
    }

    //copy resources
    dynExt(p->resourceMaps, r->resources.data, r->resources.size);

    return p;
}

void SEDestroyPipeline(SEwindow *win, SERenderPipeline *r) {
    SEVulkan *v = GetGraphics(win);
    vkDeviceWaitIdle(v->dev);

    for (u32 i = 0; i < r->bufAllocators.size; i++) {
        vkDestroyBuffer(v->dev, r->bufAllocators.data[i].b, NULL);
        FreeDeviceMem(&v->memory.heaps.data[r->bufAllocators.data[i].memid], 
                      r->bufAllocators.data[i].r);

        DestroyManager(r->bufAllocators.data[i].m);
        
    }

    dynFree(r->barriers);

    for (u32 i = 0; i < r->framebuffers.size; i++) { vkDestroyFramebuffer(v->dev, r->framebuffers.data[i], NULL); }
    dynFree(r->framebuffers);

    for (u32 i = 0; i < r->passes.size; i++) {
        vkDestroyRenderPass(v->dev, r->passes.data[i].rpass, NULL);
        vkDestroyPipeline(v->dev, r->passes.data[i].pipe, NULL);
    }
    dynFree(r->passes);

    Free(r->a, r, sizeof(SERenderPipeline));
}

void SEDestroyPipelineInfo(SEwindow *win, SERenderPipelineInfo *r) {
    SEVulkan *v = GetGraphics(win);

    for (u32 i = 0; i < r->layouts.size; i++) { vkDestroyPipelineLayout(v->dev, r->layouts.data[i], NULL); }
    dynFree(r->layouts);

    for (u32 i = 0; i < r->shaders.size; i++) { vkDestroyShaderModule(v->dev, r->shaders.data[i], NULL); }
    dynFree(r->shaders);
    dynFree(r->resources);
    dynFree(r->reads);
    dynFree(r->writes);
    dynFree(r->passes);
    dynFree(r->attrs);
    dynFree(r->bindings);
    dynFree(r->vertBufInfo);
    dynFree(r->indexBufInfo);
    Free(r->a, r, sizeof(SERenderPipelineInfo));
}

void* SEMapVertBuffer(SEwindow* win, SERenderPipeline* r, u32 resourceID) {
    SEVulkan *v = GetGraphics(win);

    u32 id = r->resourceMaps.data[resourceID].vk.buf.idx;
    SEBuffer b = r->vertexBuffers.data[id];
    u32 memid = r->bufAllocators.data[b.parent].memid;

    void* ptr;
    VkResult result = vkMapMemory(v->dev, v->memory.mem.data[memid], b.r.offset, b.r.size, 0, &ptr);
    if (result != VK_SUCCESS) return NULL;
    return ptr;
}

void SEUnMapVertBuffer(SEwindow* win, SERenderPipeline* r, u32 resourceID) {
    SEVulkan *v = GetGraphics(win);
    //Nothin for now
}

#include <string.h>

void SEUploadBuffer(SEwindow* win, SERenderPipeline* r, u32 resourceID, void* src, u32 size) {
    SEVulkan* v = GetGraphics(win);

    u32 id = r->resourceMaps.data[resourceID].vk.buf.idx;
    SEBuffer dst = r->vertexBuffers.data[id];

    u32 remaining = size;
    BufferAllocator alloc = r->bufAllocators.data[dst.parent];
    while (remaining) {
        u32 num_bytes = MIN(PAGE_SIZE, remaining);
        memcpy(v->transfer.ptr, src + (size - remaining), num_bytes); 
        
        VkCommandBufferBeginInfo info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        };
        vkQueueWaitIdle(v->queues.transfer);

        vkResetCommandBuffer(v->transfer.cmd, 0);
        vkBeginCommandBuffer(v->transfer.cmd, &info);
        VkBufferCopy cpy = {
            .size = num_bytes,
            .srcOffset = 0,
            .dstOffset = dst.r.offset + (size - remaining)
        };
        vkCmdCopyBuffer(v->transfer.cmd, v->transfer.buf, alloc.b, 1, &cpy);
        vkEndCommandBuffer(v->transfer.cmd);

        VkSubmitInfo submit = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &v->transfer.cmd,
        };
        vkQueueSubmit(v->queues.transfer, 1, &submit, VK_NULL_HANDLE);

        remaining -= num_bytes;
    }


}

void SEDrawPipeline(SEwindow *win, SERenderPipeline *r) {
    SEVulkan *v = GetGraphics(win);

    static u32 counter = 0;
    static u32 prev = 0;
    prev = counter;
    counter = (counter + 1) % v->swapchain.imgcount;

    vkWaitForFences(v->dev, 1, &v->inFlight, VK_TRUE, UINT32_MAX);
    vkResetFences(v->dev, 1, &v->inFlight);

    vkResetCommandBuffer(r->buf.data[counter], 0);

    VkCommandBufferBeginInfo begInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };
    vkBeginCommandBuffer(r->buf.data[counter], &begInfo);


    u32 idx = 0;
    VkResult result = vkAcquireNextImageKHR(v->dev, v->swapchain.swap, UINT32_MAX, v->imgAvalible.data[counter], NULL, &idx);

    if (result != VK_SUCCESS) {
        debuglog("%d", result);
        panic();
    }

    for (u32 i = 0; i < r->passes.size; i++) {
        SEPass pass = r->passes.data[i];
        u32 frame = pass.framebuffer;
        if (pass.targetSwap)
            frame += idx;
        VkRenderPassBeginInfo rbeg = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = pass.rpass,
            .renderArea =
                {
                    .offset = {0.0f, 0.0f},
                    .extent = {v->swapchain.width, v->swapchain.height},
                },
            .framebuffer = r->framebuffers.data[frame],
            .clearValueCount = 1,
            .pClearValues = &(VkClearValue){{{0.0f, 0.0f, 0.0f, 1.0f}}},
        };

        vkCmdBeginRenderPass(r->buf.data[counter], &rbeg, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(r->buf.data[counter], VK_PIPELINE_BIND_POINT_GRAPHICS, pass.pipe);

        VkViewport view = {
            .x = 0.0f,
            .y = 0.0f,
            .width = (float)v->swapchain.width,
            .height = (float)v->swapchain.height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        vkCmdSetViewport(r->buf.data[counter], 0, 1, &view);

        VkRect2D scissor = {
            .offset = {0.0f, 0.0f},
            .extent = {(float)v->swapchain.width, (float)v->swapchain.height},
        };
        vkCmdSetScissor(r->buf.data[counter], 0, 1, &scissor);

        ScratchArena a = ScratchArenaGet(NULL);

        VkBuffer* vertexBuffers = ArenaAlloc(&a.arena, sizeof(VkBuffer) * pass.vertbufs.size);
        VkDeviceSize* offsets = ArenaAlloc(&a.arena, sizeof(VkDeviceSize) * pass.vertbufs.size);

        //bind vertex buffers
        for (u32 i = 0; i < pass.vertbufs.size; i++) {
            SEBuffer vert = r->vertexBuffers.data[i + pass.vertbufs.start];
            vertexBuffers[i] = r->bufAllocators.data[vert.parent].b;
            offsets[i] = vert.r.offset;
        }
        vkCmdBindVertexBuffers(r->buf.data[counter], 0, pass.vertbufs.size, vertexBuffers, offsets);
        ScratchArenaEnd(a);

        pass.func(&(SECmdBuf){r->buf.data[counter]}, &pass);

        vkCmdEndRenderPass(r->buf.data[counter]);
    }

    vkCmdPipelineBarrier(r->buf.data[counter],
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // src stage
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,             // dst stage
                         0,                                             // dependence flags
                         0,                                             // memory barrier count
                         NULL,                                          // memory barriers
                         0,                                             // buffer count
                         NULL,                                          // buffer barriers
                         1,                                             // img barrier count
                         &(VkImageMemoryBarrier){
                             .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                             .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                             .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                             .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                             .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                             .image = v->swapchain.imgs[idx],
                             .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                             .dstAccessMask = 0,
                             .subresourceRange =
                                 {
                                     .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                     .baseArrayLayer = 0,
                                     .baseMipLevel = 0,
                                     .layerCount = 1,
                                     .levelCount = 1,
                                 },
                         });

    vkEndCommandBuffer(r->buf.data[counter]);

    VkPipelineStageFlagBits dstStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo subInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &r->buf.data[counter],
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &v->imgAvalible.data[counter],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &v->renderfinished.data[idx],
        .pWaitDstStageMask = &dstStages,
    };

    vkQueueSubmit(v->queues.graphics, 1, &subInfo, v->inFlight);

    VkPresentInfoKHR present = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &v->renderfinished.data[idx],
        .swapchainCount = 1,
        .pSwapchains = &v->swapchain.swap,
        .pImageIndices = &idx,
    };
    vkQueuePresentKHR(v->queues.present, &present);
}
