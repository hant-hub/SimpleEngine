#ifndef SE_RENDER_PIPELINE_H
#define SE_RENDER_PIPELINE_H

#include "render/cache.h"
#include <render/render.h>
#include <vulkan/vulkan.h>
#include <util.h>
#include <vulkan/vulkan_core.h>
#include <render/vertex.h>
#include <render/memory.h>

typedef struct SE_render_pass {
    VkRenderPass rp;
} SE_render_pass;


typedef enum SE_pass_type {
    SE_OPAQUE_NO_DEPTH,
} SE_pass_type;

typedef struct SE_sub_pass {
    SE_pass_type type;
    u32 vert;
    u32 shader;
    u32 start;
    u32 num; //If Num == 0 that signifies a new renderpass
} SE_sub_pass;

/*
 * INFO(ELI): For now this represents a
 * subpass. Future render pass presets will contain
 * a number of these in addition to attachments.
 * 
 * The goal is an API for constructing pipelines which
 * is similar to that of sb.h build (version 2).
 */

typedef enum SE_attach_usage {
    SE_attach_color,
    SE_attach_depth,
    SE_attach_input,
} SE_attach_usage;

typedef struct SE_subpass_attach {
    SE_attach_usage usage;
    u32 attach_idx;
} SE_subpass_attach;

typedef enum SE_attachment_type {
    SE_SwapImg,
    SE_DepthImg,
    SE_ColorImg,
} SE_attachment_type;

typedef struct SE_shaders {
    VkShaderModule vert;
    VkShaderModule frag;
    VkDescriptorSet set;
    VkPipelineLayout layout;
    u32 pipelineIdx;
} SE_shaders;

typedef struct SE_sync_objs {
    VkSemaphore* avalible;
    VkSemaphore* finished;
    VkFence* pending;
    u32 numFrames;
} SE_sync_objs;

typedef struct SE_render_pipeline {
    //backing memory for the pipeline
    SE_allocator mem;
    SE_resource_arena backingmem;

    VkPipeline* pipelines;

    SE_sync_objs s;

    VkFramebuffer* framebuffers;
    SE_render_pass* rpasses;

    //Attachments
    VkImage* imgs;
    VkImageView* views;
    SE_attachment_type* attach_types;

    u32 num_attach;

    u32 numframebuffers;
    u32 numSubpasses;
    u32 numPasses;
} SE_render_pipeline;


typedef struct SE_render_pipeline_info {

    SE_attachment_type* attachments; 
    u32 atsize;
    u32 atcap;

    SE_vertex_spec* verts;
    u32 vsize;
    u32 vcap;

    SE_shaders* shaders;
    u32 ssize;
    u32 scap;

    SE_sub_pass* passes;
    u32 psize;
    u32 pcap;

    SE_subpass_attach* attach_refs;
    u32 rsize;
    u32 rcap;

} SE_render_pipeline_info;



SE_shaders SE_LoadShaders(SE_render_context* r, const char* vert, const char* frag);
SE_sync_objs SE_CreateSyncObjs(const SE_render_context* r);

//Pipeline Creation
SE_render_pipeline_info SE_BeginPipelineCreation(void);
SE_render_pipeline SE_EndPipelineCreation(SE_render_context* r, SE_render_pipeline_info* info, const SE_pipeline_cache* c);

//Add resource
u32 SE_AddShader(SE_render_pipeline_info* p, SE_shaders* s, u32 pipeline);
u32 SE_AddVertSpec(SE_render_pipeline_info* p, SE_vertex_spec* v);

u32 SE_AddDepthAttachment(SE_render_pipeline_info* p);

//Passes
void SE_OpqaueNoDepthPass(SE_render_pipeline_info* p, u32 vert, u32 target, u32 shader);
void SE_OpqauePass(SE_render_pipeline_info* p, u32 vert, u32 target, u32 depth, u32 shader);

//Resize and Draw
void SE_CreateFrameBuffers(const SE_render_context* r, SE_render_pipeline* p);
void SE_DrawFrame(SE_window* win, SE_render_context* r, SE_render_pipeline* p, SE_render_buffer* vert);

#endif
