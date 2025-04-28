#include "generated/generated_types.h"
#include "platform.h"
#include "util.h"
#include <render/vertex.h>
#include <vulkan/vulkan_core.h>
#include <generated/generated_static.h>



SE_vertex_spec SE_CreateVertSpecInline(SE_mem_arena* a, SE_struct_member* mem, u64 size) {
    SE_vertex_spec spec = {0};  

    spec.numattrs = size;
    spec.attrs = SE_ArenaAlloc(a, sizeof(VkVertexInputAttributeDescription) * size);
    spec.bindings = SE_ArenaAlloc(a, sizeof(VkVertexInputBindingDescription));

    u64 totalsize = 0;
    for (u64 i = 0; i < size; i++) {
        spec.attrs[i] = (VkVertexInputAttributeDescription){
           .binding = 0,
           .location = i,
           .offset = mem[i].offset,
        };
        totalsize += mem[i].size;

        switch (mem[i].type) {
            case Meta_Type_f32:
                {
                    SE_Log("float: %s\n", mem[i].name);
                    spec.attrs[i].format = VK_FORMAT_R32_SFLOAT;
                    break;
                }
            case Meta_Type_SE_v2f:
                {
                    SE_Log("vec2: %s\n", mem[i].name);
                    spec.attrs[i].format = VK_FORMAT_R32G32_SFLOAT;
                    break;
                }
            case Meta_Type_SE_v3f:
                {
                    SE_Log("vec3: %s\n", mem[i].name);
                    spec.attrs[i].format = VK_FORMAT_R32G32B32_SFLOAT;
                    break;
                }
            case Meta_Type_SE_v4f:
                {
                    spec.attrs[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
                    break;
                }
            default:
                {
                    SE_Log("Invalid Member In Vertex: %s\n", mem[i].name);
                    SE_Exit(-1);
                }
        }
    }

    spec.bindings[0] = (VkVertexInputBindingDescription) {
        .binding = 0,
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        .stride = totalsize
    };

    spec.inputstate = (VkPipelineVertexInputStateCreateInfo){
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexAttributeDescriptionCount = size,
        .vertexBindingDescriptionCount = 1,
        .pVertexAttributeDescriptions = spec.attrs,
        .pVertexBindingDescriptions = spec.bindings,
    };

    SE_Log("Spec Created\n");

    return spec;
}
