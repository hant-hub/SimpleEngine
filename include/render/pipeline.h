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

/*
 * INFO(ELI): For now this represents a
 * subpass. Future render pass presets will contain
 * a number of these in addition to attachments.
 * 
 * The goal is an API for constructing pipelines which
 * is similar to that of sb.h build (version 2).
 */
typedef enum SE_pass_type {
    SE_NO_DEPTH_OPAQUE_PASS,
    SE_OPAQUE_PASS,
} SE_pass_type;

typedef enum SE_attachment_type {
    SE_SWAPCHAIN_IMG,
    SE_DEPTH_ATTACHMENT,
    SE_COLOR_ATTACHMENT,
} SE_attachment_type;

typedef struct SE_attachment_request {
    SE_attachment_type t;
} SE_attachment_request;

typedef struct SE_attachment_refs {
    VkAttachmentDescription* descrip;
    VkAttachmentReference* ref;
    u32 num;
} SE_attachment_refs;

typedef struct SE_attachments {
    VkImage* img;
    VkImageView* view;

    SE_resource_arena backingmem;
    u32 num;
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
void SE_CreateFrameBuffers(SE_render_context* r, SE_render_pipeline* p);

void SE_DrawFrame(SE_window* win, SE_render_context* r, SE_render_pipeline* p, SE_sync_objs* s, SE_resource_arena* vert);

#endif
