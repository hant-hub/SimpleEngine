#include "RenderContext.h"
#include "platform/platform.h"
#include "platform/wayland/wayland.h"
#include "utils.h"
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_wayland.h>


static const char* ValidationLayers[] = {
    "VK_LAYER_KHRONOS_validation"
};
static const uint32_t ValidationCount = ASIZE(ValidationLayers);

static const char* DeviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

static uint32_t DeviceExtensionCount = ASIZE(DeviceExtensions);




static int32_t rateDevice(VkPhysicalDevice p, VkSurfaceKHR surf, uint32_t* gfam, uint32_t* pfam) {
    uint32_t score = 0;

    {
        uint32_t famCount;
        vkGetPhysicalDeviceQueueFamilyProperties(p, &famCount, NULL);

        VkQueueFamilyProperties qprops[famCount];
        vkGetPhysicalDeviceQueueFamilyProperties(p, &famCount, qprops);

        {
            Bool32 graphics = FALSE; 
            Bool32 present = FALSE; 
            for (int j = 0; j < famCount; j++) {
                VkBool32 presentSupport;
                vkGetPhysicalDeviceSurfaceSupportKHR(p, j, surf, &presentSupport); 
                if (presentSupport) {
                    *pfam = j;
                    graphics = TRUE;
                }
                if (qprops[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    *gfam = j;
                    present = TRUE;
                }
                if (graphics && present) break;
            }

            if (!present || !graphics) {
                return -1;
            }
        }
    }
    
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(p, NULL, &extensionCount, NULL);

        VkExtensionProperties extensions[extensionCount];
        vkEnumerateDeviceExtensionProperties(p, NULL, &extensionCount, extensions);

        for (int i = 0; i < DeviceExtensionCount; i++) {
            Bool32 found = FALSE;
            for (int j = 0; j < extensionCount; j++) {
                if (strcmp(DeviceExtensions[i], extensions[j].extensionName) == 0) {
                    found = TRUE;
                    break;
                }
            }
            if (!found) return -1;
        }
    }
    {
        VkPhysicalDeviceProperties props;
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceProperties(p, &props);
        vkGetPhysicalDeviceFeatures(p, &features);

        switch (props.deviceType) {
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                {
                    score += 5;
                    break;
                }
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                {
                    score += 4;
                    break;
                }
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                {
                    score += 3;
                    break;
                }
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                {
                    score += 2;
                    break;
                }
            case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                {
                    score += 1;
                    break;
                }
            default:
                {
                    break;
                }
        }
    }


    return score;
}


void SE_InitSwapChain(SE_RenderContext* r, SE_window* win) {
    static Bool32 startup = TRUE;
    
    //runs once for the first call
    if (startup) { 
        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(r->pdevice, r->surface, &formatCount, NULL);
        VkSurfaceFormatKHR formats[formatCount];
        vkGetPhysicalDeviceSurfaceFormatsKHR(r->pdevice, r->surface, &formatCount, formats);

        //choose format
        r->swap.format = formats[0]; //default
        for (int i = 0; i < formatCount; i++) {
            if (formats[i].format == VK_FORMAT_R8G8B8A8_SRGB &&
                    formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                r->swap.format = formats[i];
            }
        }

        uint32_t modeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(r->pdevice, r->surface, &modeCount, NULL);
        VkPresentModeKHR modes[modeCount];
        vkGetPhysicalDeviceSurfacePresentModesKHR(r->pdevice, r->surface, &modeCount, modes);

        //choose mode
        r->swap.mode = VK_PRESENT_MODE_FIFO_KHR;
        for (int i = 0; i < modeCount; i++) {
            if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
                r->swap.mode = modes[i];
            }
        }

        startup = FALSE;
    } else {
        r->swap.format = r->swap.format;
        r->swap.mode = r->swap.mode;
    }

    VkSurfaceCapabilitiesKHR surfcap;  
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(r->pdevice, r->surface, &surfcap);

    //Compute Frame Size
    if (surfcap.currentExtent.width != UINT32_MAX) {
        r->swap.extent = surfcap.currentExtent;
    } else {
        r->swap.extent.width  = 
            CLAMP(surfcap.maxImageExtent.width,   surfcap.minImageExtent.width,   win->status.width);

        r->swap.extent.height = 
            CLAMP(surfcap.maxImageExtent.height,  surfcap.minImageExtent.height,  win->status.height);
    }
    
    //TODO(ELI): The number of images created can be greater than the minimum
    //Therefore we need to query how many images the swapchain created
    uint32_t minimages = surfcap.minImageCount + 1;
    if (surfcap.maxImageCount && r->swap.imgCount > surfcap.maxImageCount) {
        minimages = surfcap.maxImageCount;
    }

    uint32_t numqueues = 2;
    VkSharingMode mode = VK_SHARING_MODE_CONCURRENT;
    if (r->ldev.gfam == r->ldev.pfam) {
        numqueues = 1;
        mode = VK_SHARING_MODE_EXCLUSIVE;
    }

    uint32_t queues[2] = {r->ldev.gfam, r->ldev.pfam};

    VkSwapchainCreateInfoKHR swapInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = r->surface,
        .imageExtent = r->swap.extent,
        .minImageCount = minimages,
        .imageFormat = r->swap.format.format,
        .imageColorSpace = r->swap.format.colorSpace,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .queueFamilyIndexCount = numqueues,
        .pQueueFamilyIndices = queues,
        .imageSharingMode = mode,
        .preTransform = surfcap.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .clipped = TRUE,
        //.oldSwapchain = r->swap.swap //TODO(ELI): Figure out if this matters
    };

    ZERO_CHECK(vkCreateSwapchainKHR(r->ldev.l, &swapInfo, NULL, &r->swap.swap));

    vkGetSwapchainImagesKHR(r->ldev.l, r->swap.swap, &r->swap.imgCount, NULL);
    r->swap.imgs = SE_Alloc(sizeof(VkImage) * r->swap.imgCount);
    r->swap.views = SE_Alloc(sizeof(VkImageView) * r->swap.imgCount);
    vkGetSwapchainImagesKHR(r->ldev.l, r->swap.swap, &r->swap.imgCount, r->swap.imgs);

    for (int i = 0; i < r->swap.imgCount; i++) {
        VkImageViewCreateInfo viewInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .format = r->swap.format.format,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .image = r->swap.imgs[i],
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        ZERO_CHECK(vkCreateImageView(r->ldev.l, &viewInfo, NULL, &r->swap.views[i]));
    }
}

void SE_DestroySwapChain(SE_RenderContext* r) {
    for (int i = 0; i < r->swap.imgCount; i++) {
        vkDestroyImageView(r->ldev.l, r->swap.views[i], NULL);
        //vkDestroyImage(r->ldev.l, r->swap.imgs[i], NULL);
    }
    vkDestroySwapchainKHR(r->ldev.l, r->swap.swap, NULL);
}


SE_RenderContext SE_CreateRenderContext(SE_window* win) {
    SE_RenderContext r = {0};

    //instance
    {
        VkApplicationInfo appInfo = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_API_VERSION_1_0,
            .engineVersion = VK_MAKE_VERSION(1, 0 ,0),
            .pApplicationName = "test",
            .pEngineName = "No Engine",
        };


        VkInstanceCreateInfo instanceInfo = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &appInfo,
            .enabledExtensionCount = InstanceExtensionCount,
            .ppEnabledExtensionNames = InstanceExtensions,
            .enabledLayerCount = ValidationCount,
            .ppEnabledLayerNames = ValidationLayers,
        };

        ZERO_CHECK(vkCreateInstance(&instanceInfo, NULL, &r.instance));
    }

    //surface
    ZERO_CHECK(SE_CreateSurface(r.instance, win, &r.surface));

    //Physical Device
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(r.instance, &deviceCount, NULL);

        VkPhysicalDevice devices[deviceCount];
        vkEnumeratePhysicalDevices(r.instance, &deviceCount, devices);

        {
            int32_t maxscore = 0; 
            for (int i = 0; i < deviceCount; i++) {
                uint32_t gfam;
                uint32_t pfam;
                int32_t score = rateDevice(devices[i], r.surface, &gfam, &pfam);

                if (score > maxscore) {
                    maxscore = score;
                    r.pdevice = devices[i];
                    r.ldev.gfam = gfam;
                    r.ldev.pfam = pfam;
                }

            }
            ZERO_CHECK(!r.pdevice);
        }
    }

    //logical device
    {
        float priority = 1.0f; //only single queue for now
        VkDeviceQueueCreateInfo queueInfos[2]; 
        queueInfos[0] = (VkDeviceQueueCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pQueuePriorities = &priority,
                .queueCount = 1,
                .queueFamilyIndex = r.ldev.gfam,
        };
        queueInfos[1] = (VkDeviceQueueCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pQueuePriorities = &priority,
                .queueCount = 1,
                .queueFamilyIndex = r.ldev.pfam,
        };

        uint32_t numqueues = 2;
        if (r.ldev.pfam == r.ldev.gfam) {
            numqueues = 1;
        }

        VkPhysicalDeviceFeatures devFeatures = {0};
        VkDeviceCreateInfo devInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount = numqueues,
            .pQueueCreateInfos = queueInfos,
            .enabledLayerCount = ValidationCount,
            .ppEnabledLayerNames = ValidationLayers,
            .enabledExtensionCount = DeviceExtensionCount,
            .ppEnabledExtensionNames = DeviceExtensions,
        };
        ZERO_CHECK(vkCreateDevice(r.pdevice, &devInfo, NULL, &r.ldev.l));
        vkGetDeviceQueue(r.ldev.l, r.ldev.gfam, 0, &r.ldev.g);
        vkGetDeviceQueue(r.ldev.l, r.ldev.pfam, 0, &r.ldev.p);
    }

    //initialize swapchain
    SE_InitSwapChain(&r, win);

    return r;
}

SE_ShaderProg SE_LoadShader(SE_RenderContext* r, const char* vert, const char* frag) {
    SE_ShaderProg s = {0};

    Buffer vbuf = SE_LoadFile(vert);
    Buffer fbuf = SE_LoadFile(frag);

    VkShaderModuleCreateInfo vertInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = vbuf.len,
        .pCode = (uint32_t*)vbuf.data,
    };

    VkShaderModuleCreateInfo fragInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = fbuf.len,
        .pCode = (uint32_t*)fbuf.data,
    };

    ZERO_CHECK(vkCreateShaderModule(r->ldev.l, &vertInfo, NULL, &s.vert));
    ZERO_CHECK(vkCreateShaderModule(r->ldev.l, &fragInfo, NULL, &s.frag));

    return s;
}

void SE_UnLoadShader(SE_RenderContext* r, SE_ShaderProg* s) {
    vkDestroyShaderModule(r->ldev.l, s->vert, NULL);
    vkDestroyShaderModule(r->ldev.l, s->frag, NULL);
}


SE_Pipeline SE_CreatePipeline(SE_ShaderProg* s) {
    SE_Pipeline p;

    




    return p;
}
