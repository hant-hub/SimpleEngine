#ifndef SE_RENDER_UTILS_H
#define SE_RENDER_UTILS_H
#include "vulkan/vulkan_core.h"
#include <render/render.h>


//pick format out of a list,
//prefers formats which come earlier
VkFormat SE_FindSupportedFormat( SE_render_context* r, const VkFormat* const candidates,
                                 u32 num_formats, VkImageTiling tiling,
                                 VkFormatFeatureFlags flags);



#endif
