#ifndef VERTEX_H
#define VERTEX_H

#include <vulkan/vulkan_core.h>
#include <generated/generated_static.h>
#include <util.h>

typedef struct SE_vertex_spec {
    VkPipelineVertexInputStateCreateInfo inputstate;
    VkVertexInputAttributeDescription* attrs;
    VkVertexInputBindingDescription* bindings;
    u32 numattrs;
    u32 numbindings;
} SE_vertex_spec;

#endif
