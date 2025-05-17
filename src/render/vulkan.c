#include "platform.h"
#include "util.h"
#include <render/render.h>
#include <stdint.h>
#include <stdio.h>
#include <vulkan/vulkan_core.h>

#include "pipeline.c"
#include "memory.c"
#include "vertex.c"
#include "utils.c"

static const char* InstanceExtensions[] = {
    VK_KHR_SURFACE_EXTENSION_NAME
};
static const u32 InstanceExtensionCount = ASIZE(InstanceExtensions);


#ifdef SE_DEBUG_VULKAN
static const char* ValidationLayers[] = {
    "VK_LAYER_KHRONOS_validation"
};
static const u32 ValidationLayerCount = ASIZE(ValidationLayers);
#endif


static const char* DeviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    //VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME,
};

static const u32 DeviceExtensionCount = ASIZE(DeviceExtensions);



i32 RatePhysicalDevice(SE_allocator* a, u32* gfam, u32* pfam, VkPhysicalDevice p, VkSurfaceKHR surf) {
    i32 score = 0;

    //Extension support
    u32 numExtensions;
    vkEnumerateDeviceExtensionProperties(p, NULL, &numExtensions, NULL);
    VkExtensionProperties* extprops = a->alloc(0, (sizeof(VkExtensionProperties) * numExtensions), NULL, a->ctx);
    assert(extprops);
    vkEnumerateDeviceExtensionProperties(p, NULL, &numExtensions, extprops);

    for (u32 i = 0; i < DeviceExtensionCount; i++) {
        Bool32 found = FALSE; 
        for (u32 j = 0; j < numExtensions; j++) {
            if (SE_strcmp(DeviceExtensions[i], extprops[j].extensionName)) {
                found = TRUE;
            }
        }
        if (!found) {
            SE_Log("Missing Extensions\n");
            return -1;
        }
    }

    //present support
    u32 numfam;
    vkGetPhysicalDeviceQueueFamilyProperties(p, &numfam, NULL);

    VkQueueFamilyProperties* qprops = a->alloc(0, sizeof(VkQueueFamilyProperties) * numfam, NULL, a->ctx);
    vkGetPhysicalDeviceQueueFamilyProperties(p, &numfam, qprops);

    Bool32 pfound = FALSE;
    Bool32 gfound = FALSE;
    for (int i = 0; i < numfam; i++) {
        VkBool32 supported;
        vkGetPhysicalDeviceSurfaceSupportKHR(p, i, surf, &supported);
        if (supported) {
            *pfam = i;
            pfound = TRUE;
        }
        if (qprops[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            *gfam = i;
            gfound = TRUE;
        }
        if (gfound && pfound) break;

    }

    if (!pfound || !gfound) {
        return -1;
    }

    VkPhysicalDeviceProperties prop;
    vkGetPhysicalDeviceProperties(p, &prop);

    switch (prop.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            {
                score += 4;
                break;
            }
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            {
                score += 3;
                break;
            }
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            {
                score += 2;
                break;
            }
        default:
            {
                score += 1;
            }
    }

    return score;
}

SE_swapchain SE_CreateSwapChain(SE_allocator* a, SE_render_context* r, SE_window* win, SE_swapchain* old) {
    SE_swapchain s = {0};
    //SwapChain
    
    SE_mem_arena* t;
    if (!a) {
        t = SE_HeapArenaCreate(4096 * 5); 
        a->ctx = &t;
        a->alloc = SE_StaticArenaAlloc;
    }

    {
        u32 numformats;
        REQUIRE_ZERO(vkGetPhysicalDeviceSurfaceFormatsKHR(r->p, r->surf, &numformats, NULL));
        VkSurfaceFormatKHR* formats = a->alloc(0, sizeof(VkSurfaceFormatKHR) * numformats, NULL, a->ctx);
        REQUIRE_ZERO(vkGetPhysicalDeviceSurfaceFormatsKHR(r->p, r->surf, &numformats, formats));

        u32 numModes;
        REQUIRE_ZERO(vkGetPhysicalDeviceSurfacePresentModesKHR(r->p, r->surf, &numModes, NULL));
        VkPresentModeKHR* modes = a->alloc(0, sizeof(VkPresentModeKHR) * numModes, NULL, a->ctx);
        REQUIRE_ZERO(vkGetPhysicalDeviceSurfacePresentModesKHR(r->p, r->surf, &numModes, modes));

        //pick format
        s.format = formats[0];
        for (u32 i = 0; i < numformats; i++) {
            if (formats[i].format == VK_FORMAT_R8G8B8A8_SRGB && 
                    formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                s.format = formats[i];
            }
        }
        //check if undefined
        assert(s.format.format);

        //pick mode
        s.mode = VK_PRESENT_MODE_FIFO_KHR;
        for (u32 i = 0; i < numModes; i++) {
            if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
                s.mode = modes[i];
            }
        }
        //check, should never equal 0 since it is initialized to FIFO and 
        //can only be updated to Mailbox
        assert(s.mode);

        //free mem
        a->alloc(sizeof(VkSurfaceFormatKHR) * numformats, 0, formats, a->ctx);
        a->alloc(sizeof(VkPresentModeKHR) * numModes, 0, modes, a->ctx);
    }
    {
        VkSurfaceCapabilitiesKHR cap;
        VkResult code = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(r->p, r->surf, &cap);

        if (code == VK_ERROR_SURFACE_LOST_KHR) {
            SE_Log("Lost Surface\n");
        }

        VkExtent2D extent;
        if (cap.currentExtent.width && 
            cap.currentExtent.height &&
            cap.currentExtent.width != UINT32_MAX &&
            cap.currentExtent.height != UINT32_MAX
            ) {
            extent = cap.currentExtent;
        } else {
            extent.width = CLAMP(cap.maxImageExtent.width, cap.minImageExtent.width, win->width);
            extent.height = CLAMP(cap.maxImageExtent.height, cap.minImageExtent.height, win->height);
        }

        u32 minImages = cap.minImageCount + 1;
        minImages = (minImages > cap.maxImageCount) && cap.maxImageCount ? cap.maxImageCount : minImages;

        SE_Log("Frame Extent: (%d, %d)\n", extent.width, extent.height);
        s.size = extent;


        //create swapchain
        VkSharingMode sharing = r->Queues.gfam == r->Queues.pfam ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
        u32 indicies[] = {r->Queues.gfam, r->Queues.pfam};

        VkSwapchainCreateInfoKHR swapInfo = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .presentMode = s.mode,
            .imageFormat = s.format.format,
            .imageColorSpace = s.format.colorSpace,
            .imageArrayLayers = 1,
            .imageExtent = extent,
            .clipped = VK_TRUE,
            .preTransform = cap.currentTransform,
            .imageSharingMode = sharing,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .surface = r->surf,
            .minImageCount = minImages,
            .queueFamilyIndexCount = r->Queues.gfam == r->Queues.pfam ? 1 : 2,
            .pQueueFamilyIndices = indicies,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .oldSwapchain = old->swap
        };

        REQUIRE_ZERO(vkCreateSwapchainKHR(r->l, &swapInfo, NULL, &s.swap));
    }
    if (old->swap) {
        for (u32 i = 0; i < old->numImgs; i++) {
            vkDestroyImageView(r->l, old->views[i], NULL);
        }

        //images are destroyed by swapchain
        vkDestroySwapchainKHR(r->l, old->swap, NULL);
        SE_HeapFree(old->views);
        SE_HeapFree(old->imgs);
    }

    //images and views
    {
        vkGetSwapchainImagesKHR(r->l, s.swap, &s.numImgs, NULL);
        s.imgs = SE_HeapAlloc(sizeof(VkImage) * s.numImgs);
        vkGetSwapchainImagesKHR(r->l, s.swap, &s.numImgs, s.imgs);

        s.views = SE_HeapAlloc(sizeof(VkImageView) * s.numImgs);
        for (u32 i = 0; i < s.numImgs; i++) {
            VkImageViewCreateInfo viewInfo = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .components = {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
                .format = s.format.format,
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .layerCount = 1,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .baseMipLevel = 0,
                },
                .image = s.imgs[i],
            };

            REQUIRE_ZERO(vkCreateImageView(r->l, &viewInfo, NULL, &s.views[i]));
        }
    }

    if (t) {
        SE_HeapFree(t);
    }

    SE_Log("SwapChain with %d Images Created\n", s.numImgs);
    return s;
}


SE_render_context SE_CreateRenderContext(SE_window* win) {
    SE_render_context rc = {0};
    //TODO(ELI): Run Tests To Make Sure this is enough
    //memory for initializing vulkan, it should be,
    //but check to make sure
    SE_mem_arena* m = SE_HeapArenaCreate((4096 * 32));
    SE_allocator a = (SE_allocator){
        .alloc = SE_StaticArenaAlloc,
        .ctx = m
    };

    {

        u32 numExtensions = InstanceExtensionCount + SE_PlatformInstanceExtensionCount; 
        const char** Extensions = a.alloc(0, sizeof(char*) * numExtensions, NULL, a.ctx);

        for (u32 i = 0; i < InstanceExtensionCount; i++) {
            Extensions[i] = InstanceExtensions[i];
        }
        for (u32 i = 0; i < SE_PlatformInstanceExtensionCount; i++) {
            Extensions[i + InstanceExtensionCount] = SE_PlatformInstanceExtensions[i];
        }

        VkApplicationInfo appInfo = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = win->title,
            .pEngineName = "Simple Engine",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_API_VERSION_1_0
        };

        VkInstanceCreateInfo instanceInfo = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &appInfo,
            .ppEnabledExtensionNames = Extensions,
            .enabledExtensionCount = numExtensions,
            .enabledLayerCount = ValidationLayerCount,
            .ppEnabledLayerNames = ValidationLayers,
        };

        REQUIRE_ZERO(vkCreateInstance(&instanceInfo, NULL, &rc.instance));

        m->size = 0;
    }

    rc.surf = SE_CreateVKSurface(win, rc.instance);

    {
        u32 numDevices;        
        vkEnumeratePhysicalDevices(rc.instance, &numDevices, NULL);

        //there should be at least one device
        assert(numDevices);

        VkPhysicalDevice* devices = a.alloc(0, sizeof(VkPhysicalDevice) * numDevices, NULL, a.ctx);
        vkEnumeratePhysicalDevices(rc.instance, &numDevices, devices); 

        i32 maxscore = 0;
        SE_mem_arena* a1 = SE_HeapArenaCreate((sizeof(VkQueueFamilyProperties) * 5000));
        SE_allocator a = (SE_allocator){
            .alloc = SE_StaticArenaAlloc,
            .ctx = a1
        };
        for (u32 i = 0; i < numDevices; i++) {
            u32 gfam, pfam;
            i32 score = RatePhysicalDevice(&a, &gfam, &pfam, devices[i], rc.surf);
            if (score >= maxscore) {
                rc.p = devices[i];
                rc.Queues.gfam = gfam;
                rc.Queues.pfam = pfam;
                maxscore = score;
            }
            a1->size = 0;
        }
        m->size = 0;
        assert(rc.p);
        SE_Log("Physical Device Created!\n");
        SE_Log("Queue Families: %u %u\n", rc.Queues.gfam, rc.Queues.pfam);
    }

    //Logical Device
    {
        float priority = 1.0f;
        u32 numQueues = 1;
        VkDeviceQueueCreateInfo queueInfos[2];
        queueInfos[0] = (VkDeviceQueueCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueCount = 1,
                .queueFamilyIndex = rc.Queues.gfam,
                .pQueuePriorities = &priority,
        };

        if (rc.Queues.gfam != rc.Queues.pfam) {
            ++numQueues;
            queueInfos[1] = (VkDeviceQueueCreateInfo){
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .queueCount = 1,
                    .pQueuePriorities = &priority,
                    .queueFamilyIndex = rc.Queues.pfam,
            };
        }

        VkPhysicalDeviceFeatures features = {0};

        VkDeviceCreateInfo devInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount = numQueues,
            .pQueueCreateInfos = queueInfos,
            .pEnabledFeatures = &features,
            .enabledExtensionCount = DeviceExtensionCount,
            .ppEnabledExtensionNames = DeviceExtensions,
            .enabledLayerCount = ValidationLayerCount,
            .ppEnabledLayerNames = ValidationLayers,
        };

        REQUIRE_ZERO(vkCreateDevice(rc.p, &devInfo, NULL, &rc.l));
        vkGetDeviceQueue(rc.l, rc.Queues.gfam, 0, &rc.Queues.g);
        vkGetDeviceQueue(rc.l, rc.Queues.pfam, 0, &rc.Queues.p);
    }

    m->size = 0;
    rc.s = SE_CreateSwapChain(&a, &rc, win, &rc.s); 

    {
        //command buffer 
        VkCommandPoolCreateInfo poolInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = rc.Queues.gfam,
            .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        };

        REQUIRE_ZERO(vkCreateCommandPool(rc.l, &poolInfo, NULL, &rc.pool));
        SE_Log("Command Pool Created\n");

        VkCommandBufferAllocateInfo cmdAlloc = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandBufferCount = 1,
            .commandPool = rc.pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        };

        REQUIRE_ZERO(vkAllocateCommandBuffers(rc.l, &cmdAlloc, &rc.cmd));
    }

    rc.m = SE_CreateHeapTrackers(&rc);
    SE_HeapFree(m);
    return rc;
}

