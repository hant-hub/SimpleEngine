#include "core/cutils.h"
#include "ds/ds.h"
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
        //g->swapchain.mode = VK_PRESENT_MODE_FIFO_KHR;
        
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
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, 
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

    if (feats.samplerAnisotropy) {
        score += 20;
    }

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

u32 AddShader(SEwindow* win, SString filename) {
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

    VkShaderModule mod = CompileShader(v, buffer);
    dynPush(v->shaders, mod);
    ScratchArenaEnd(sc);

    return v->shaders.size - 1;
}


//Platform agnostic vulkan code
void CreateVulkan(VkInstance inst, VkSurfaceKHR surf, SEwindow* win, SEVulkan* g, SEsettings* settings) {
    //We can pass inst as a value since it is technically a
    //pointer so passing it by pointer would be a double indirection and
    //save no space

    Allocator a = win->mem;
    g->inst = inst;
    g->surf = surf;

    g->imgAvalible.a = a;
    g->renderfinished.a = a;
    g->memory.heaps.a = a;
    g->memory.mem.a = a;
    g->memory.types.a = a;
    g->memory.props.a = a;
    g->shaders.a = a;

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
        
        //set important feature flags
        g->pdev = dev;

        {
            VkPhysicalDeviceFeatures feats;
            vkGetPhysicalDeviceFeatures(g->pdev, &feats);

            if (feats.samplerAnisotropy) {
                g->features.anisotropy = TRUE;
            }
        }


        u32 familyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(dev, &familyCount, NULL); 
        VkQueueFamilyProperties* props = StackAlloc(s, familyCount * sizeof(VkQueueFamilyProperties));
        vkGetPhysicalDeviceQueueFamilyProperties(dev, &familyCount, props); 

        debuglog("Num Families: %d", familyCount);

        VkBool32 present = VK_FALSE;
        VkBool32 graph = VK_FALSE;
        VkBool32 transfer = VK_FALSE;
        for (u32 i = 0; i < familyCount; i++) {
            if (graph && present && transfer) break;
            if (!graph) {
                if (props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    g->queues.gfam = i;
                    graph = VK_TRUE;
                }
            }
            if (!transfer) {
                if (props[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
                    g->queues.tfam = i;
                    if (g->queues.tfam != g->queues.gfam) transfer = VK_TRUE;
                }
            }
            if (!present) {
                vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, surf, &present);
                if (present) g->queues.pfam = i;
            }
        }
        assert(present && graph);
        debuglog("pfam: %d, gfam: %d, tfam: %d", g->queues.pfam, g->queues.gfam, g->queues.tfam);

        StackDestroy(a, s);
    }

    //extract queues
    {
        bool8 a = g->queues.gfam != g->queues.pfam;
        bool8 b = g->queues.pfam != g->queues.tfam;
        bool8 c = g->queues.gfam != g->queues.tfam;

        u32 num_queues = 1 + (a || b || c) + (a && b);
        u32 idx = 0;

        f32 priority = 1.0f;
        ScratchArena sc = ScratchArenaGet(NULL);
        VkDeviceQueueCreateInfo* queueInfos = ArenaAlloc(&sc.arena, sizeof(VkDeviceQueueCreateInfo) * num_queues);

        queueInfos[idx++] = (VkDeviceQueueCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = g->queues.gfam,
            .queueCount = 1,
            .pQueuePriorities = &priority,
        };

        if (a) {
            queueInfos[idx++] = (VkDeviceQueueCreateInfo){
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = g->queues.pfam,
                .queueCount = 1,
                .pQueuePriorities = &priority
            };
        }

        if (b && c) {
            queueInfos[idx++] = (VkDeviceQueueCreateInfo){
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = g->queues.tfam,
                .queueCount = 1,
                .pQueuePriorities = &priority
            };
        }

        VkPhysicalDeviceFeatures feats = {
            .samplerAnisotropy = g->features.anisotropy,
        };

        VkDeviceCreateInfo devInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount = num_queues,
            .pQueueCreateInfos = queueInfos,
            .enabledLayerCount = 0,
            .ppEnabledExtensionNames = ldevExtensions,
            .enabledExtensionCount = ARRAY_SIZE(ldevExtensions),
            .pEnabledFeatures = &feats,
        };

        debuglog("Queue Num: %d", num_queues);
        vkCreateDevice(g->pdev, &devInfo, NULL, &g->dev);

        vkGetDeviceQueue(g->dev, g->queues.gfam, 0, &g->queues.graphics);
        vkGetDeviceQueue(g->dev, g->queues.pfam, 0, &g->queues.present);
        vkGetDeviceQueue(g->dev, g->queues.tfam, 0, &g->queues.transfer);

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

    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(g->pdev, &memProps);

    for (u32 i = 0; i < memProps.memoryHeapCount; i++) {
        VkMemoryHeap heap = memProps.memoryHeaps[i];
        printlog("Heap index: %d\n", i);
        printlog("\tsize: %l MB\n", heap.size/MB(1));
        printlog("\tDevice Local: %d\n", !!(heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT));
    }

    //init transfer buffer

    {
        u32 queues[] = {g->queues.gfam, g->queues.tfam};
        VkBufferCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pQueueFamilyIndices = queues,
            .queueFamilyIndexCount = 1 + (g->queues.gfam != g->queues.tfam),
            .sharingMode = (g->queues.gfam == g->queues.tfam) ?
                            VK_SHARING_MODE_EXCLUSIVE :
                            VK_SHARING_MODE_CONCURRENT,

            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .size = PAGE_SIZE,
        };

        vkCreateBuffer(g->dev, &info, NULL, &g->transfer.buf);
        VkFenceCreateInfo fenceInfo = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT,
        };
        vkCreateFence(g->dev, &fenceInfo, NULL, &g->transfer.fence);

        VkMemoryRequirements reqs;
        vkGetBufferMemoryRequirements(g->dev, g->transfer.buf, &reqs);

        VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceMemoryProperties(g->pdev, &memProps);

        u32 memid = 0;
        u32 flag = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        for (u32 i = 0; i < memProps.memoryTypeCount; i++) {
            VkMemoryType type = memProps.memoryTypes[i];

            if (type.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD) continue;
            if (type.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD) continue;
            if (!(reqs.memoryTypeBits & (1 << i))) continue;

            if ((type.propertyFlags & flag) == flag) {
                memid = i;
                break;
            }
        }
        
        VkMemoryAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .memoryTypeIndex = memid,
            .allocationSize = reqs.size,
        };

        vkAllocateMemory(g->dev, &allocInfo, NULL, &g->transfer.mem);
        vkBindBufferMemory(g->dev, g->transfer.buf, g->transfer.mem, 0);
        vkMapMemory(g->dev, g->transfer.mem, 0, PAGE_SIZE, 0, &g->transfer.ptr);


    }

    //Allocate Command Pools
    {
        //Transfer Queue
        VkCommandPoolCreateInfo poolInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = g->queues.tfam,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        };
        vkCreateCommandPool(g->dev, &poolInfo, NULL, &g->transfer.pool);
        
        VkCommandBufferAllocateInfo cmdInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = g->transfer.pool,
            .commandBufferCount = 1,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        };
        vkAllocateCommandBuffers(g->dev, &cmdInfo, &g->transfer.cmd);

        //Graphics Queue
        poolInfo.queueFamilyIndex = g->queues.gfam;
        vkCreateCommandPool(g->dev, &poolInfo, NULL, &g->graphics.pool);
        cmdInfo.commandPool = g->graphics.pool;
        vkAllocateCommandBuffers(g->dev, &cmdInfo, &g->graphics.cmd);

        //Present Queue
        poolInfo.queueFamilyIndex = g->queues.pfam;
        vkCreateCommandPool(g->dev, &poolInfo, NULL, &g->present.pool);
        cmdInfo.commandPool = g->present.pool;
        vkAllocateCommandBuffers(g->dev, &cmdInfo, &g->present.cmd);
    }

    //handle settings
    {
        //default settings
        SEsettings s = {};
        if (settings) {
            s = *settings;
        }

        //default memory
        if (s.memory.max_static_mem == 0) {
            s.memory.max_static_mem = MB(40);
        }

        if (s.memory.max_dynamic_mem == 0) {
            s.memory.max_dynamic_mem = MB(20);
        }

        //TODO(ELI): default size stuff later
        SEConfigMaxGPUMem(win, SE_MEM_STATIC, s.memory.max_static_mem);
        SEConfigMaxGPUMem(win, SE_MEM_DYNAMIC, s.memory.max_dynamic_mem);
    }

    //configure feature info
    {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(g->pdev, &props);
        g->featureInfo.anisotropy.max = props.limits.maxSamplerAnisotropy; 
    }
}


void DestroyVulkan(SEVulkan g, Allocator a) {
    vkDeviceWaitIdle(g.dev);

    vkDestroyBuffer(g.dev, g.transfer.buf, NULL);
    vkFreeMemory(g.dev, g.transfer.mem, NULL);

    //for (u32 i = 0; i < g.bufAllocators.size; i++) {
    //    vkDestroyBuffer(g.dev, g.bufAllocators.data[i].b, NULL);
    //    DestroyManager(g.bufAllocators.data[i].m);
    //}
    //dynFree(g.bufAllocators);
    
    for (u32 i = 0; i < g.memory.heaps.size; i++) {
        DestroyManager(g.memory.heaps.data[i]);
        vkFreeMemory(g.dev, g.memory.mem.data[i], NULL);
    }
    dynFree(g.memory.heaps);
    dynFree(g.memory.mem);
    dynFree(g.memory.types);
    dynFree(g.memory.props);

    for (u32 i = 0; i < g.swapchain.imgcount; i++) {
        vkDestroySemaphore(g.dev, g.imgAvalible.data[i], NULL);
        vkDestroySemaphore(g.dev, g.renderfinished.data[i], NULL);
    }
    vkDestroyFence(g.dev, g.inFlight, NULL);
    vkDestroyFence(g.dev, g.transfer.fence, NULL);
    dynFree(g.imgAvalible);
    dynFree(g.renderfinished);
    
    for (u32 i = 0; i < g.shaders.size; i++) {
        vkDestroyShaderModule(g.dev, g.shaders.data[i], NULL);
    }
    dynFree(g.shaders);

    vkDestroyCommandPool(g.dev, g.pool, NULL);
    vkDestroyCommandPool(g.dev, g.transfer.pool, NULL);
    vkDestroyCommandPool(g.dev, g.graphics.pool, NULL);
    vkDestroyCommandPool(g.dev, g.present.pool, NULL);

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
