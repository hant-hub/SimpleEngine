#ifndef SE_RENDER_H
#define SE_RENDER_H

#include "util.h"
#include <platform.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

typedef struct SE_render_heap {
    VkMemoryPropertyFlagBits props;
    u64 top;
    u64 size;
} SE_render_heap;

typedef struct SE_render_memory {
    SE_render_heap* heaps;
    u32 numheaps;
} SE_render_memory;

//TODO(ELI): API independent way to represent
//Rendering pipeline, ie: renderpasses, subpasses,
//attachments

//NOTE(ELI): RenderPasses only render from attachments, to
//attachments, so only the subpasses in a renderpass need
//to reference each other, and the renderpasses themselves can
//be completely decoupled.


//IDEA: Store the subpasses in a stack style array,
//renderpass is a range into that array, with the
//attachments being similar. Each Renderpass can
//be stored in its own array as well.


//Actual Swapchain allocation
//will need to be recomputed with
//window resize
typedef struct SE_swapchain {
    VkSwapchainKHR swap;
    VkSurfaceFormatKHR format;
    VkPresentModeKHR mode;
    VkExtent2D size;
    u32 numImgs;
    VkImage* imgs;
    VkImageView* views;
} SE_swapchain;


typedef struct SE_render_context {
    VkInstance instance;
    VkSurfaceKHR surf;
    VkDevice l;
    VkPhysicalDevice p;
    VkCommandPool pool;
    VkCommandBuffer cmd;

    //Optimal tiling, I don't think linear tiling will
    //be necessary
    
    //For copies and CPU uploads just use a VkBuffer

    VkFormat depthFormat;
    VkFormat stencilFormat;
    VkFormat depthStencilFormat;

    struct Queues {
        u32 gfam;
        u32 pfam;
        VkQueue g;
        VkQueue p;
    } Queues;

    SE_swapchain s;
    SE_render_memory m;
} SE_render_context;

#endif
