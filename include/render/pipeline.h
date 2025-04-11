#ifndef SE_RENDER_PIPELINE_H
#define SE_RENDER_PIPELINE_H

#include <render/render.h>
#include <vulkan/vulkan.h>
#include <util.h>
#include <vulkan/vulkan_core.h>
#include <render/vertex.h>
#include <render/memory.h>

typedef struct SE_render_pass {
    VkRenderPass rp;
    u32 numSubPasses;
    u32 startidx;
} SE_render_pass;

typedef struct SE_sub_pass {
    VkSubpassDescription descrip;
    VkSubpassDependency dep;
    u32 colorAttachment;
    u32 depthAttachment;
} SE_sub_pass;

typedef struct SE_attachment_refs {
    VkAttachmentDescription* descrip;
    VkAttachmentReference* ref;
    u32 num;
} SE_attachment_refs;

typedef struct SE_attachments {
    VkImage* img;
    VkImageView* view;
    u32 num;
    u32 cap;
} SE_attachments;

typedef struct SE_shaders {
    VkShaderModule vert;
    VkShaderModule frag;
    VkDescriptorSet set;
    VkPipelineLayout layout;
} SE_shaders;

typedef struct SE_sub_pass_config {
    VkSubpassDescription des;
    VkSubpassDependency dep;
} SE_sub_pass_config;

typedef struct SE_pass_config {
   u32 numSubPasses; 
   u32 numAttachments;
   SE_sub_pass_config* subconfigs;
   VkAttachmentDescription* attachments;
} SE_pass_config;

typedef struct SE_render_pipeline {
    //backing memory for the pipeline
    SE_mem_arena mem;

    VkPipeline* pipelines;
    SE_shaders* shaders;

    VkFramebuffer* framebuffers;

    SE_render_pass* rpasses;
    SE_sub_pass* subpasses;

    SE_attachment_refs refs;
    SE_attachments attachments;

    u32 numframebuffers;
    u32 numPasses;
} SE_render_pipeline;

typedef struct SE_sync_objs {
    VkSemaphore* avalible;
    VkSemaphore* finished;
    VkFence* pending;
    u32 numFrames;
} SE_sync_objs;


SE_shaders SE_LoadShaders(SE_render_context* r, const char* vert, const char* frag);
SE_sync_objs SE_CreateSyncObjs(SE_render_context* r);

SE_render_pipeline SE_CreatePipeline(SE_mem_arena a, SE_render_context* r, SE_vertex_spec* vspec, SE_shaders* s);

void SE_DrawFrame(SE_window* win, SE_render_context* r, SE_render_pipeline* p, SE_sync_objs* s, SE_resource_arena* vert);

#endif
