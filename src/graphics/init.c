#include "cutils.h"
#include "ds.h"
#include "se.h"
#include "vulkan/vulkan_core.h"
#include <assert.h>
#include <graphics/graphics_intern.h>
#include <string.h>

static const char* ldevExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

VKAPI_ATTR VkResult VKAPI_CALL (*CreateDebugMessenger)(
    VkInstance                                  instance,
    const VkDebugUtilsMessengerCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugUtilsMessengerEXT*                   pMessenger);

VKAPI_ATTR void VKAPI_CALL (*DestroyDebugMessenger)(
    VkInstance                                  instance,
    VkDebugUtilsMessengerEXT                    messenger,
    const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    const VkDebugUtilsMessageSeverityFlagsEXT severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;

    if (messageSeverity >= severity) {
        switch (messageSeverity) {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT: break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                printlog("Validation Layer: %n\n", pCallbackData->pMessage);
                return VK_FALSE;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                printwarn("\033[38;2;255;255;0mValidation Layer: %n\033[0m\n", pCallbackData->pMessage);
                return VK_FALSE;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                printerr("\033[38;2;255;0;0mValidation Layer: %n\033[0m\n", pCallbackData->pMessage);
                return VK_FALSE;
        }
    }
    return VK_FALSE;
}

void LoadExtensionFuncs(VkInstance instance) {
    CreateDebugMessenger = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    assert(CreateDebugMessenger);

    DestroyDebugMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    assert(DestroyDebugMessenger);
}

//also doubles as recreate swapchain function
void CreateSwapChain(SEwindow* win, SEVulkan* g, Allocator a) {
        SwapChainInfo* details = &g->swapInfo;


        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g->pdev, g->surf, &details->cap);

        u32 prevformats = details->numformats;
        vkGetPhysicalDeviceSurfaceFormatsKHR(g->pdev, g->surf, &details->numformats, NULL);
        assert(details->numformats);

        details->formats = Realloc(a, details->formats,
                                   prevformats * sizeof(VkSurfaceFormatKHR),
                                   details->numformats * sizeof(VkSurfaceFormatKHR)); 

        vkGetPhysicalDeviceSurfaceFormatsKHR(g->pdev, g->surf, &details->numformats, details->formats);

        u32 prevModes = details->nummodes;
        vkGetPhysicalDeviceSurfacePresentModesKHR(g->pdev, g->surf, &details->nummodes, NULL);
        assert(details->nummodes);

        details->modes = Realloc(a, details->modes,
                                 prevModes * sizeof(VkPresentModeKHR),
                                 details->nummodes * sizeof(VkPresentModeKHR));

        vkGetPhysicalDeviceSurfacePresentModesKHR(g->pdev, g->surf, &details->nummodes, details->modes);

        //pick format
        g->swapchain.format = details->formats[0];
        for (u32 i = 0; i < details->numformats; i++) {
            if (details->formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
                details->formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                g->swapchain.format = details->formats[i];
                break;
            }
        }

        //pick present mode
        for (u32 i = 0; i < details->nummodes; i++) {
            if (details->modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
                g->swapchain.mode = VK_PRESENT_MODE_MAILBOX_KHR;
            }
        }
        g->swapchain.mode = VK_PRESENT_MODE_FIFO_KHR;
        
        debuglog("Window Dimensions: (%d, %d)", win->width, win->height);
        debuglog("Window Mode: %d", g->swapchain.mode);

        g->swapchain.width = CLAMP(details->cap.minImageExtent.width, details->cap.maxImageExtent.width, win->width); 
        g->swapchain.height = CLAMP(details->cap.minImageExtent.height, details->cap.maxImageExtent.height, win->height); 

        u32 mincount = details->cap.minImageCount + 1;
        if (details->cap.maxImageCount > 0) {
            mincount = MIN(mincount, details->cap.maxImageCount);
        }

        VkSwapchainKHR oldswap = g->swapchain.swap;
        VkSwapchainCreateInfoKHR swapinfo = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = g->surf,
            .minImageCount = mincount,
            .imageFormat = g->swapchain.format.format,
            .imageColorSpace = g->swapchain.format.colorSpace,
            .imageExtent = (VkExtent2D){g->swapchain.width, g->swapchain.height},
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .preTransform = g->swapInfo.cap.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = g->swapchain.mode,
            .clipped = VK_TRUE,
            .oldSwapchain = oldswap,
        };

        u32 queues[2] = {g->queues.gfam, g->queues.pfam};
        if (g->queues.gfam != g->queues.pfam) {
            swapinfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapinfo.queueFamilyIndexCount = 2;
            swapinfo.pQueueFamilyIndices = queues;
        } else {
            swapinfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        vkCreateSwapchainKHR(g->dev, &swapinfo, NULL, &g->swapchain.swap);
        if (oldswap) vkDestroySwapchainKHR(g->dev, oldswap, NULL);

        //grab swap images + views
        u32 oldcount = g->swapchain.imgcount;

        for (u32 i = 0; i < oldcount; i++) {
            vkDestroyImageView(g->dev, g->swapchain.views[i], NULL);
        }


        vkGetSwapchainImagesKHR(g->dev, g->swapchain.swap, &g->swapchain.imgcount, NULL);
        
        g->swapchain.imgs = Realloc(a, g->swapchain.imgs,
                                    oldcount * sizeof(VkImage),
                                    g->swapchain.imgcount * sizeof(VkImage));

        vkGetSwapchainImagesKHR(g->dev, g->swapchain.swap, &g->swapchain.imgcount, g->swapchain.imgs);

        g->swapchain.views = Realloc(a, g->swapchain.views,
                                    oldcount * sizeof(VkImageView),
                                    g->swapchain.imgcount * sizeof(VkImageView));

        for (u32 i = 0; i < g->swapchain.imgcount; i++) {
            VkImageViewCreateInfo viewInfo = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = g->swapchain.imgs[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = g->swapchain.format.format,
                .components = {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            };

            vkCreateImageView(g->dev, &viewInfo, NULL, &g->swapchain.views[i]);
        }
}



u32 PhysicalDeviceScore(VkPhysicalDevice p, VkSurfaceKHR surf, SwapChainInfo* details, StackAllocator s, Allocator a) {
    u32 score = 1;
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(p, &props);

    VkPhysicalDeviceFeatures feats;
    vkGetPhysicalDeviceFeatures(p, &feats);

    if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 100;
    }
    score += props.limits.maxImageDimension2D;

    u32 familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(p, &familyCount, NULL); 

    VkBool32 presentSupport = VK_FALSE;
    for (u32 i = 0; i < familyCount; i++) {
        vkGetPhysicalDeviceSurfaceSupportKHR(p, i, surf, &presentSupport);
        if (presentSupport) break;
    }

    //check extension support

    u32 extnum;
    vkEnumerateDeviceExtensionProperties(p, NULL, &extnum, NULL);
    VkExtensionProperties *extprops = StackAlloc(s, extnum * sizeof(VkExtensionProperties));
    assert(extprops);
    vkEnumerateDeviceExtensionProperties(p, NULL, &extnum, extprops);


    VkBool32 extensionSupport = VK_TRUE; 
    for (u32 i = 0; i < ARRAY_SIZE(ldevExtensions); i++) {
        const char* tocheck = ldevExtensions[i];
        bool8 found = FALSE;
        for (u32 j = 0; j < extnum; j++) {
            if (strncmp(tocheck, extprops[j].extensionName, sizeof(extprops[j].extensionName)) == 0) {
                found = TRUE;
                break;
            }
        }
        if (!found) {
            extensionSupport = VK_FALSE;
            break;
        }

    }
    
    VkBool32 swapSupport = VK_TRUE;

    vkGetPhysicalDeviceSurfaceFormatsKHR(p, surf, &details->numformats, NULL);
    swapSupport *= !!details->numformats;

    vkGetPhysicalDeviceSurfacePresentModesKHR(p, surf, &details->nummodes, NULL);
    swapSupport *= !!details->nummodes;

    //multiplying by support will disable devices which cannot present
    return score * presentSupport * extensionSupport * swapSupport;//0 is unsuitable, anything else is suitable
}


//Platform agnostic vulkan code
void CreateVulkan(VkInstance inst, VkSurfaceKHR surf, SEwindow* win, SEVulkan* g) {
    //We can pass inst as a value since it is technically a
    //pointer so passing it by pointer would be a double indirection and
    //save no space

    Allocator a = win->mem;
    g->inst = inst;
    g->surf = surf;

    g->resources.shaders.a = a;
    g->resources.layouts.a = a;
    g->resources.pipelines.a = a;
    g->imgAvalible.a = a;
    g->renderfinished.a = a;

    VkDebugUtilsMessengerCreateInfoEXT messengerInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           //VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = &debugCallback,
        .pUserData = NULL
    };

    LoadExtensionFuncs(g->inst); 
    #ifdef DEBUG
    CreateDebugMessenger(g->inst, &messengerInfo, NULL, &g->debugMessenger);
    #endif

    //pick physical device
    {
        StackAllocator s = StackCreate(a, KB(1));
        u32 devicecount = 0;
        vkEnumeratePhysicalDevices(g->inst, &devicecount, NULL);

        assert(devicecount); //must be at least one device which supports vulkan

        VkPhysicalDevice* devices = StackAlloc(s, devicecount * sizeof(VkPhysicalDevice));
        vkEnumeratePhysicalDevices(g->inst, &devicecount, devices);

        u32 maxscore = 0;
        VkPhysicalDevice dev = VK_NULL_HANDLE;
        StackAllocator scores = StackCreate(a, KB(75)); //lookup at some point, this is kinda huge
        for (u32 i = 0; i < devicecount; i++) {
            u32 score = PhysicalDeviceScore(devices[i], g->surf, &g->swapInfo, scores, a);
            StackReset(scores);
            if (score > maxscore) {
                dev = devices[i];
                maxscore = score;
            }
        }
        StackDestroy(a, scores);
        assert(maxscore); //we found a suitable device
        g->pdev = dev;

        u32 familyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(dev, &familyCount, NULL); 
        VkQueueFamilyProperties* props = StackAlloc(s, familyCount * sizeof(VkQueueFamilyProperties));
        vkGetPhysicalDeviceQueueFamilyProperties(dev, &familyCount, props); 

        VkBool32 present = VK_FALSE;
        VkBool32 graph = VK_FALSE;
        for (u32 i = 0; i < familyCount; i++) {
            if (graph && present) break;
            if (!graph) {
                if (props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    g->queues.gfam = i;
                    graph = VK_TRUE;
                }
            }
            if (!present) vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, surf, &present);
            if (present) g->queues.pfam = i;
        }
        assert(present && graph);
        debuglog("pfam: %d, gfam: %d", g->queues.pfam, g->queues.gfam);

        StackDestroy(a, s);
    }

    //extract queues
    {

        f32 priority = 1.0f;
        VkDeviceQueueCreateInfo queueInfos[2] = {
            (VkDeviceQueueCreateInfo){
               .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
               .queueFamilyIndex = g->queues.gfam,
               .queueCount = 1,
               .pQueuePriorities = &priority,
            },
            (VkDeviceQueueCreateInfo){
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = g->queues.pfam,
                .queueCount = 1,
                .pQueuePriorities = &priority
            },
        };

        VkDeviceCreateInfo devInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount = 1 + (g->queues.gfam != g->queues.pfam),
            .pQueueCreateInfos = queueInfos,
            .enabledLayerCount = 0,
            .ppEnabledExtensionNames = ldevExtensions,
            .enabledExtensionCount = ARRAY_SIZE(ldevExtensions),
        };

        vkCreateDevice(g->pdev, &devInfo, NULL, &g->dev);

        vkGetDeviceQueue(g->dev, g->queues.gfam, 0, &g->queues.graphics);
        vkGetDeviceQueue(g->dev, g->queues.pfam, 0 + (g->queues.gfam != g->queues.pfam), &g->queues.present);
    }

    //swapchain stuff
    CreateSwapChain(win, g, a);

    //command pool
    VkCommandPoolCreateInfo pool = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = g->queues.gfam,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
    };
    vkCreateCommandPool(g->dev, &pool, NULL, &g->pool);


    //fences + semaphores
    VkSemaphoreCreateInfo semInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    for (u32 i = 0; i < g->swapchain.imgcount; i++) {
        VkSemaphore imgAvalible;
        VkSemaphore renderFinished;

        vkCreateSemaphore(g->dev, &semInfo, NULL, &imgAvalible);
        vkCreateSemaphore(g->dev, &semInfo, NULL, &renderFinished);

        dynPush(g->imgAvalible, imgAvalible);
        dynPush(g->renderfinished, renderFinished);
    }
    vkCreateFence(g->dev, &fenceInfo, NULL, &g->inFlight);
}


void DestroyVulkan(SEVulkan g, Allocator a) {
    vkDeviceWaitIdle(g.dev);

    for (u32 i = 0; i < g.swapchain.imgcount; i++) {
        vkDestroySemaphore(g.dev, g.imgAvalible.data[i], NULL);
        vkDestroySemaphore(g.dev, g.renderfinished.data[i], NULL);
    }
    vkDestroyFence(g.dev, g.inFlight, NULL);
    dynFree(g.imgAvalible);
    dynFree(g.renderfinished);
    
    for (u32 i = 0; i < g.resources.shaders.cap; i++) {
        vkDestroyShaderModule(g.dev, g.resources.shaders.slots[i], NULL);
    }
    ShaderPoolDestroy(&g.resources.shaders);

    for (u32 i = 0; i < g.resources.layouts.cap; i++) {
        vkDestroyPipelineLayout(g.dev, g.resources.layouts.slots[i], NULL);
    }
    LayoutPoolDestroy(&g.resources.layouts);

    for (u32 i = 0; i < g.resources.pipelines.cap; i++) {
        vkDestroyPipeline(g.dev, g.resources.pipelines.slots[i], NULL);
    }
    PipelinePoolDestroy(&g.resources.pipelines);

    vkDestroyCommandPool(g.dev, g.pool, NULL);

    for (u32 i = 0; i < g.swapchain.imgcount; i++) {
        vkDestroyImageView(g.dev, g.swapchain.views[i], NULL);
    }
    vkDestroySwapchainKHR(g.dev, g.swapchain.swap, NULL);


    Free(a, g.swapchain.views, g.swapchain.imgcount * sizeof(VkImageView));
    Free(a, g.swapchain.imgs, g.swapchain.imgcount * sizeof(VkImage));

    vkDestroyDevice(g.dev, NULL);

    Free(a, g.swapInfo.formats, g.swapInfo.numformats * sizeof(VkSurfaceFormatKHR));
    Free(a, g.swapInfo.modes, g.swapInfo.nummodes * sizeof(VkPresentModeKHR));


    vkDestroySurfaceKHR(g.inst, g.surf, NULL);

    #ifdef DEBUG
    DestroyDebugMessenger(g.inst, g.debugMessenger, NULL);
    #endif
    vkDestroyInstance(g.inst, NULL);

}
