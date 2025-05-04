#include "render/render.h"
#include "vulkan/vulkan_core.h"
#include <render/utils.h>


VkFormat SE_FindSupportedFormat( SE_render_context* r, const VkFormat* const candidates,
                                 u32 num_formats, VkImageTiling tiling,
                                 VkFormatFeatureFlags flags) {

    for (u32 i = 0; i < num_formats; i++) {
        VkFormatProperties prop;
        vkGetPhysicalDeviceFormatProperties(r->p, candidates[i], &prop);

        VkFormatFeatureFlags features; 

        if (tiling == VK_IMAGE_TILING_LINEAR) features = prop.linearTilingFeatures;
        else features = prop.optimalTilingFeatures;

        if ((features & flags) == flags) {
            return candidates[i];
        }
    }

    return VK_FORMAT_UNDEFINED;
}
