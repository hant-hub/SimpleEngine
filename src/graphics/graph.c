// #include "cutils.h"
// #include "ds.h"s
// #include "se.h"
// #include "vulkan/vulkan_core.h"
#include <graphics/graphics_intern.h>

SERenderPipelineInfo *SECreateRenderPipeline(SEwindow *win) {
    SEVulkan *v = GetGraphics(win);
    SERenderPipelineInfo *r = Alloc(win->mem, sizeof(SERenderPipelineInfo));
    *r = (SERenderPipelineInfo){
        .passes.a = win->mem,
        .resources.a = win->mem,
        .pipeline.a = win->mem,
    };

    return r;
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

u32 SEAddPipeline(SEwindow *win, SERenderPipelineInfo *r) {
    SEVulkan *v = GetGraphics(win);
    dynPush(r->pipeline, (PipelineInfo){0});
    dynBack(r->pipeline) = (PipelineInfo){
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
    // dynPush(r->resources.data[resourceID].writes, pass);
}

void SESetBackBuffer(SERenderPipelineInfo *r, u32 resourceID) { r->backbuffer = resourceID; }

void CreateBuffer(SEwindow *win, SERenderPipelineInfo *info, SERenderPipeline *pipe, u32 resource);
void CreateImage(SEwindow *win, SERenderPipelineInfo *info, SERenderPipeline *pipe, u32 resource);
BufAllocType GetBufAlloc(SEMemType mem, VkBufferUsageFlags usage);

void BuildPass(SEwindow *win, SERenderPipelineInfo *info, SERenderPipeline *pipe, u32 pass);

void BuildFrameBuffers(SEVulkan *v, SERenderPipeline *pipe);

SERenderPipeline *SECompilePipeline(SEwindow *win, SERenderPipelineInfo *info) {
    SEVulkan *v = GetGraphics(win);

    SERenderPipeline *pipe = Alloc(win->mem, sizeof(SERenderPipeline));
    *pipe = (SERenderPipeline){
        .backbuffer = info->backbuffer,
        .bufAllocators.a = win->mem,
        .resourceMapping.a = win->mem,
        .images.a = win->mem,
        .imgInfos.a = win->mem,
        .vertBuffers.a = win->mem,
        .passes.a = win->mem,
        .passVertMapping.a = win->mem,
        .framebuffers.a = win->mem,
        .framebufferInfos.a = win->mem,
        .frameBufferMapping.a = win->mem,
    };

    // init BufferAllocators
    u32 vert_static = 0;
    u32 vert_dyn = 0;
    for (u32 i = 0; i < info->resources.size; i++) {
        if (info->resources.data[i].type != RESOURCE_BUFFER)
            continue;

        struct SEBufferInfo buf = info->resources.data[i].resourceInfo.buf;
        BufAllocType type = GetBufAlloc(buf.memType, buf.usage);

        switch (type) {
            case BUF_ALLOC_VERT_STATIC: vert_static += buf.size; break;
            case BUF_ALLOC_VERT_DYN: vert_dyn += buf.size; break;
            default:
                debugerr("Invalid Buffer Allocator");
                panic();
                break;
        }
    }

    dynResize(pipe->bufAllocators, BUF_ALLOC_NUM);

    if (vert_static)
        pipe->bufAllocators.data[BUF_ALLOC_VERT_STATIC]
            = SEConfigBufType(win, SE_BUFFER_VERT, SE_MEM_STATIC, vert_static);
    if (vert_dyn)
        pipe->bufAllocators.data[BUF_ALLOC_VERT_DYN] = SEConfigBufType(win, SE_BUFFER_VERT, SE_MEM_DYNAMIC, vert_dyn);

    // build Resources
    for (u32 i = 0; i < info->resources.size; i++) {
        switch (info->resources.data[i].type) {
            case RESOURCE_BUFFER: CreateBuffer(win, info, pipe, i); break;
            case RESOURCE_IMAGE: CreateImage(win, info, pipe, i); break;
        }
    }
    pipe->backbuffer = pipe->resourceMapping.data[pipe->backbuffer];

    // build RenderPasses
    for (u32 i = 0; i < info->passes.size; i++) { BuildPass(win, info, pipe, i); }

    BuildFrameBuffers(v, pipe);

    return pipe;
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
    final.pass.pipeline = CreatePipeline(v, final.pass.pass, &info->pipeline.data[pass->pipeline]);

    ScratchArenaEnd(sc);
}

void BuildFrameBuffers(SEVulkan *v, SERenderPipeline *pipe) {

    ScratchArena sc = ScratchArenaGet(NULL);

    for (u32 i = 0; i < pipe->passes.size; i++) {
        FrameBufferInfo *frame = &pipe->framebufferInfos.data[pipe->passes.data[i].pass.framebuffer];
        VkImageView *views = ArenaAlloc(&sc.arena, sizeof(VkImageView) * frame->num);

        for (u32 j = 0; j < frame->num; j++) views[j] = pipe->images.data[j + frame->first].view;

        VkFramebufferCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pAttachments = views,
            .attachmentCount = frame->num,
            .width = v->swapchain.width,
            .height = v->swapchain.height,
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

void CreateBuffer(SEwindow *win, SERenderPipelineInfo *info, SERenderPipeline *pipe, u32 resource) {
    struct SEBufferInfo bufInfo = info->resources.data[resource].resourceInfo.buf;

    BufAllocType t = GetBufAlloc(bufInfo.memType, bufInfo.usage);
    SEBuffer buffer = AllocBuffer(win, &pipe->bufAllocators.data[t], t, bufInfo.size);

    if (t == BUF_ALLOC_VERT_DYN || t == BUF_ALLOC_VERT_STATIC) {
        dynPush(pipe->resourceMapping, pipe->vertBuffers.size);
        dynPush(pipe->vertBuffers, buffer);
    }
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
    } else {
        if (usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
            return BUF_ALLOC_VERT_DYN;
    }

    return BUF_ALLOC_INVALID;
}
