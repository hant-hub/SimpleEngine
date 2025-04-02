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

typedef struct SE_render_pipeline {
    VkPipeline* pipelines;
    VkFramebuffer* framebuffers;
    SE_render_pass* rpasses;
    SE_sub_pass* subpasses;
    SE_attachment_ref* refs;
    SE_attachment* attachments;
} SE_render_pipeline;

typedef struct SE_shaders {
    VkShaderModule vert;
    VkShaderModule frag;
} SE_shaders;

