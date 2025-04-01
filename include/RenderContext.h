#ifndef SE_RENDER_CONTEXT_H
#define SE_RENDER_CONTEXT_H

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "platform/platform.h"

//Global or unique vulkan stuff
typedef struct RenderContext {
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice pdevice;
    struct SE_ldev {
        VkDevice l;
        VkQueue g;
        VkQueue p;
        uint32_t gfam;
        uint32_t pfam;
    } ldev;
    struct SE_swap {
        VkSwapchainKHR swap;
        VkExtent2D extent;
        VkSurfaceFormatKHR format;
        VkPresentModeKHR mode;
        VkImage* imgs;
        VkImageView* views;
        uint32_t imgCount;
    } swap;
} SE_RenderContext;


typedef struct SE_ShaderProg {
    VkShaderModule vert;
    VkShaderModule frag;
} SE_ShaderProg;


typedef struct SE_PipelineConfig {

} SE_PipelineConfig;

typedef struct SE_Pipeline {
    VkPipeline pipe;

    //NOTE(ELI): This structure is supposed
    //to contain all the data required for
    //Draw Commands, minus a command buffer.
    //This means attachments, shader buffers,
    //Descriptor sets, etc.
} SE_Pipeline;

SE_RenderContext SE_CreateRenderContext(SE_window* win);
void SE_InitSwapChain(SE_RenderContext* r, SE_window* win);
void SE_DestroySwapChain(SE_RenderContext* r);


SE_ShaderProg SE_LoadShader(SE_RenderContext* r, const char* vert, const char* frag);
void SE_UnLoadShader(SE_RenderContext* r, SE_ShaderProg* s);




#endif
