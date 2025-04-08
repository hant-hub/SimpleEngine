#ifndef SE_RENDER_PIPELINE_H
#define SE_RENDER_PIPELINE_H

#include <render/render.h>
#include <vulkan/vulkan.h>
#include <util.h>
#include <vulkan/vulkan_core.h>

typedef struct SE_render_pass {
    VkRenderPass rp;
    u32 numSubPasses;
    u32 startidx;
} SE_render_pass;

typedef struct SE_sub_pass {
    VkSubpassDescription descrip;
    VkSubpassDependency dep;
    u32 numAttachments;
    u32 startidx;
} SE_sub_pass;

typedef struct SE_attachment_ref {
    VkAttachmentDescription descrip;
    VkAttachmentReference ref;
} SE_attachment_ref;

typedef struct SE_attachment {
    VkImage img;
    VkImageView view;
} SE_attachment;

typedef struct SE_shaders {
    VkShaderModule vert;
    VkShaderModule frag;
    VkDescriptorSet set;
    VkPipelineLayout layout;
} SE_shaders;


typedef struct SE_render_pipeline {
    VkPipeline* pipelines;
    SE_shaders* shaders;

    VkFramebuffer* framebuffers;

    SE_render_pass* rpasses;
    SE_sub_pass* subpasses;
    SE_attachment_ref* refs;

    SE_attachment* attachments;

    VkSemaphore* avalible;
    VkSemaphore* finished;
    VkFence* pending;

    u32 numframebuffers;
    u32 numPasses;
    u32 numAttachments;
    u32 numFrames;
} SE_render_pipeline;


SE_shaders SE_LoadShaders(SE_render_context* r, const char* vert, const char* frag);
SE_render_pipeline SE_CreatePipeline(SE_render_context* r, SE_shaders* s);

void SE_DrawFrame(SE_window* win, SE_render_context* r, SE_render_pipeline* p);

#endif
