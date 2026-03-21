#include "cutils.h"
#include "ds.h"
#include "graphics/graphics.h"
#include "vulkan/vulkan_core.h"

#include <graphics/graphics_intern.h>
#include <core/introspect.h>



void CreateDescriptorLayout(SEVulkan* v) {

    VkDescriptorPoolSize size = {
        .descriptorCount = 1,
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    };

    VkDescriptorPoolCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = 1,
        .pPoolSizes = &size,
        .maxSets = 1,
    };

}
