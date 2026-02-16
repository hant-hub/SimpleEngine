#include "cutils.h"
#include "ds.h"
#include "se.h"
#include "vulkan/vulkan_core.h"
#include <graphics/graphics_intern.h>

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
    };

    for (u32 i = 0; i < v->swapchain.imgcount; i++) {
        dynPush(r->resources, ((Resource){
                                  .lastUsage = RESOURCE_UNINITIALIZED,
                                  .type = COLOR_ATTACHMENT,
                                  .clear = TRUE,
                                  .swapchain = TRUE,
                                  .vk.view = v->swapchain.views[i],
                              }));
    }

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

u32 SEAddResource(SERenderPipelineInfo *r, bool8 clear) {
    dynPush(r->resources, ((Resource){.lastUsage = RESOURCE_READ, .type = COLOR_ATTACHMENT, .clear = clear}));
    return r->resources.size - 1;
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
    dynArray(VkAttachmentDescription) descrips;
    dynArray(VkAttachmentReference) refs;
    dynArray(VkSubpassDependency) deps;
    dynArray(VkImageView) views;
} PipelineBuildInfo;

void ProcessReads(PipelineBuildInfo *buildInfo, SERenderPassInfo *passInfo, SERenderPipelineInfo *r) {
    u32 offset = passInfo->readStart;
    for (u32 j = 0; j < passInfo->numReads; j++) {
        // update resource tracker
        // Access = 0 barrier
        // If last was read no barrier needed
        // If Last was a write then we need an invalidate barrier,
        // wait until resource is done being written to, before we
        // read it. The stage to read and to write depends on the resource

        Resource *obj = &r->resources.data[r->reads.data[offset + j]];

        dynPush(buildInfo->views, obj->vk.view);
        VkAttachmentDescription descrip = {};

        VkAttachmentReference ref = {
            .attachment = buildInfo->descrips.size,
        };

        VkSubpassDependency dep = {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
        };

        descrip.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        switch (obj->lastUsage) {
            case RESOURCE_UNINITIALIZED: {
                panic(); // should never happen
            } break;
            case RESOURCE_READ: {
                if (obj->type == COLOR_ATTACHMENT) {
                    descrip.format = VK_FORMAT_B8G8R8A8_SRGB;
                    descrip.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    descrip.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    descrip.samples = VK_SAMPLE_COUNT_1_BIT;

                    descrip.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

                    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                }           
            } break;
            case RESOURCE_WRITE: {
                if (obj->type == COLOR_ATTACHMENT) {
                    descrip.format = VK_FORMAT_B8G8R8A8_SRGB;
                    descrip.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    descrip.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    descrip.samples = VK_SAMPLE_COUNT_1_BIT;

                    descrip.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

                    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                    dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                    dep.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

                    dep.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

                    // NOTE(ELI): For now just block everywhere the attachment could be used
                    // namely input attachment, shader attachment, or color read.
                    dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT |
                                        VK_ACCESS_SHADER_READ_BIT;

                    dynPush(buildInfo->deps, dep);
                }
            } break;
        }

        obj->lastUsage = RESOURCE_READ;
        dynPush(buildInfo->descrips, descrip);
        dynPush(buildInfo->refs, ref);

        debuglog("\tRead: %d", r->reads.data[j]);
    }
}

bool8 ProcessWrites(PipelineBuildInfo *buildInfo, SERenderPassInfo *passInfo, SERenderPipelineInfo *r) {
    u32 offset = passInfo->writeStart;
    bool8 swapchain = FALSE;
    for (u32 j = 0; j < passInfo->numWrites; j++) {
        // update resource tracker
        // insert flush barrier
        // If a read happened before we need an execution barrier
        // to ensure the write happends after the read
        //
        // If a write happened before we need a memory barrier to
        // force the second write to happen first

        Resource *obj = &r->resources.data[r->writes.data[offset + j]];

        if (obj->swapchain)
            swapchain = TRUE;
        else
            dynPush(buildInfo->views, obj->vk.view);

        VkAttachmentDescription descrip = {};

        VkAttachmentReference ref = {
            .attachment = buildInfo->descrips.size,
        };

        VkSubpassDependency dep = {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
        };

        descrip.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        switch (obj->lastUsage) {
            case RESOURCE_UNINITIALIZED: {
                if (obj->type == COLOR_ATTACHMENT) {
                    descrip.format = VK_FORMAT_B8G8R8A8_SRGB;

                    descrip.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    descrip.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                    descrip.samples = VK_SAMPLE_COUNT_1_BIT;

                    if (obj->clear)
                        descrip.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    else
                        descrip.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

                    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                    dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

                    dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                    dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

                    dynPush(buildInfo->deps, dep);

                }
            } break;
            case RESOURCE_READ: {
                debuglog("hit");
                if (obj->type == COLOR_ATTACHMENT) {
                    descrip.format = VK_FORMAT_B8G8R8A8_SRGB;

                    descrip.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    descrip.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                    descrip.samples = VK_SAMPLE_COUNT_1_BIT;

                    descrip.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

                    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                    dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                    dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

                    dep.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT |
                                        VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;

                    dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

                    dynPush(buildInfo->deps, dep);
                }
            } break;
            case RESOURCE_WRITE: {
                debuglog("hit");
                if (obj->type == COLOR_ATTACHMENT) {
                    descrip.format = VK_FORMAT_B8G8R8A8_SRGB;
                    descrip.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                    descrip.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                    descrip.samples = VK_SAMPLE_COUNT_1_BIT;

                    descrip.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

                    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                    dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                    dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

                    dep.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

                    dynPush(buildInfo->deps, dep);
                } 
            } break;
        }

        obj->lastUsage = RESOURCE_WRITE;

        dynPush(buildInfo->descrips, descrip);
        dynPush(buildInfo->refs, ref);

        debuglog("\tWrite: %d", r->writes.data[j]);
    }
    return swapchain;
}

void BuildPass(PipelineBuildInfo *buildInfo, SEVulkan *v, SERenderPipelineInfo *r, SERenderPipeline *p,
               SERenderPassInfo *passInfo) {

    PipelineBarrier b = {};
    // additively write to barrier info

    ProcessReads(buildInfo, passInfo, r);
    bool8 swapchain = ProcessWrites(buildInfo, passInfo, r);

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
        .framebuffer = p->buffers.size,
        .targetSwap = swapchain,
        .func = (SEDrawFunc)passInfo->func,
    };
    debuglog("targetSwap: %d", swapchain);

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
        dynPush(p->buffers, buf);
    } else {
        dynPush(buildInfo->views, r->resources.data[0].vk.view);
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
            dynPush(p->buffers, buf);
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
        .buffers.a = win->mem,
    };

    PipelineBuildInfo buildInfo = {
        .descrips.a = win->mem,
        .refs.a = win->mem,
        .deps.a = win->mem,
        .views.a = win->mem,
    };

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
        .commandBufferCount = 1,
        .commandPool = v->pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    };

    vkAllocateCommandBuffers(v->dev, &bufinfo, &p->buf);
    return p;
}

void SEDestroyPipeline(SEwindow *win, SERenderPipeline *r) {
    SEVulkan *v = GetGraphics(win);
    vkDeviceWaitIdle(v->dev);

    dynFree(r->barriers);

    for (u32 i = 0; i < r->buffers.size; i++) { vkDestroyFramebuffer(v->dev, r->buffers.data[i], NULL); }
    dynFree(r->buffers);

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
    Free(r->a, r, sizeof(SERenderPipelineInfo));
}

void SEDrawPipeline(SEwindow *win, SERenderPipeline *r) {
    SEVulkan *v = GetGraphics(win);

    vkWaitForFences(v->dev, 1, &v->inFlight, VK_TRUE, UINT32_MAX);
    vkResetFences(v->dev, 1, &v->inFlight);

    vkResetCommandBuffer(r->buf, 0);

    VkCommandBufferBeginInfo begInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };
    vkBeginCommandBuffer(r->buf, &begInfo);

    static u32 counter = 0;
    counter = (counter + 1) % v->swapchain.imgcount;

    u32 idx = 0;
    assert(vkAcquireNextImageKHR(v->dev, v->swapchain.swap, UINT32_MAX, v->imgAvalible.data[counter], NULL, &idx) ==
           VK_SUCCESS);

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
            .framebuffer = r->buffers.data[frame],
            .clearValueCount = 1,
            .pClearValues = &(VkClearValue){{{0.0f, 0.0f, 0.0f, 1.0f}}},
        };

        vkCmdBeginRenderPass(r->buf, &rbeg, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(r->buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pass.pipe);

        VkViewport view = {
            .x = 0.0f,
            .y = 0.0f,
            .width = (float)v->swapchain.width,
            .height = (float)v->swapchain.height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        vkCmdSetViewport(r->buf, 0, 1, &view);

        VkRect2D scissor = {
            .offset = {0.0f, 0.0f},
            .extent = {(float)v->swapchain.width, (float)v->swapchain.height},
        };
        vkCmdSetScissor(r->buf, 0, 1, &scissor);

        pass.func(&(SECmdBuf){r->buf}, &pass);

        vkCmdEndRenderPass(r->buf);
    }

    vkCmdPipelineBarrier(r->buf,
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

    vkEndCommandBuffer(r->buf);

    VkPipelineStageFlagBits dstStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo subInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &r->buf,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &v->imgAvalible.data[counter],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &v->renderfinished.data[counter],
        .pWaitDstStageMask = &dstStages,
    };

    vkQueueSubmit(v->queues.graphics, 1, &subInfo, v->inFlight);

    VkPresentInfoKHR present = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &v->renderfinished.data[counter],
        .swapchainCount = 1,
        .pSwapchains = &v->swapchain.swap,
        .pImageIndices = &idx,
    };
    vkQueuePresentKHR(v->queues.present, &present);
}
