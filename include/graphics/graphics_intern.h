#ifndef SE_INIT_H
#define SE_INIT_H

#include "cutils.h"
#include "ds.h"
#include "vulkan/vk_platform.h"
#include "vulkan/vulkan_core.h"

#include <graphics/graphics.h>

PoolDecl(ShaderPool, VkShaderModule);
PoolDecl(LayoutPool, VkPipelineLayout);
PoolDecl(PipelinePool, VkPipeline);

//shouldn't be static since this should be shared across
//platforms

typedef struct SwapChainInfo {
    VkSurfaceCapabilitiesKHR cap;
    VkSurfaceFormatKHR* formats;
    VkPresentModeKHR* modes;
    u32 numformats;
    u32 nummodes;
} SwapChainInfo;

//vulkan
typedef struct SEVulkan {
    //graphics
    //NOTE(ELI): If I ever decide to support
    //multiwindow applications, the instance, physical device
    //and possibly the logical device need to be split into
    //a global
    VkInstance inst;
    VkPhysicalDevice pdev;
    SwapChainInfo swapInfo;
    VkDevice dev; 
    VkSurfaceKHR surf;
    VkCommandPool pool;
    VkCommandBuffer buf;

    VkSemaphore imgAvalible;
    VkSemaphore renderfinished;
    VkFence inFlight;

    struct {
        //Will be the same if
        //same queue supports both operations.
        u32 gfam;
        u32 pfam;
        VkQueue graphics;
        VkQueue present;
    } queues;

    struct {
        //NOTE(ELI): stored in context since there
        //will never be more than one swapchain per
        //window
        VkSurfaceFormatKHR format;
        VkPresentModeKHR mode;
        u32 width;
        u32 height;
        u32 imgcount;
        VkSwapchainKHR swap;

        VkImage* imgs;
        VkImageView* views;
    } swapchain;

    struct {
        ShaderPool shaders;
        LayoutPool layouts;
        PipelinePool pipelines;
    } resources;

    #ifdef DEBUG
    VkDebugUtilsMessengerEXT debugMessenger;
    #endif
} SEVulkan;


//renderpass
typedef struct SERenderPass {
    VkRenderPass pass;
    VkFramebuffer* buffers;
    u32 numBuffers;
    u32 pipeline; //one for now
} SERenderPass;


VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData); 


extern VKAPI_ATTR VkResult VKAPI_CALL (*CreateDebugMessenger)(
    VkInstance                                  instance,
    const VkDebugUtilsMessengerCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugUtilsMessengerEXT*                   pMessenger);

extern VKAPI_ATTR void VKAPI_CALL (*DestroyDebugMessenger)(
    VkInstance                                  instance,
    VkDebugUtilsMessengerEXT                    messenger,
    const VkAllocationCallbacks*                pAllocator);

void LoadExtensionFuncs(VkInstance instance);

void CreateVulkan(VkInstance inst, VkSurfaceKHR surf, SEwindow* win, SEVulkan* g);
void DestroyVulkan(SEVulkan g, Allocator a);

#endif
