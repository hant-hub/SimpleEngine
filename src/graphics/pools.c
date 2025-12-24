#include "ds.h"
#include "vulkan/vulkan_core.h"

#include <graphics/graphics_intern.h>

PoolImpl(ShaderPool, VkShaderModule);
PoolImpl(LayoutPool, VkPipelineLayout);
PoolImpl(PipelinePool, VkPipeline);
