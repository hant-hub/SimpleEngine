#include "cutils.h"
#include "ds.h"
#include "graphics/graphics.h"
#include "se.h"
#include "vulkan/vulkan_core.h"

#include <graphics/graphics_intern.h>
#include <core/introspect.h>

void AddVertexBinding(SERenderPipelineInfo* rinfo, SEBindingType type, SEStructSpec* layout, u32 numMembers) {

    //next binding, plus increment
    u32 binding = dynBack(rinfo->passes).vertInfo.numBindings++;
    u32 size = 0;

    VkVertexInputBindingDescription desc = {
        .binding = binding,
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, 
    };

    if (type == SE_BINDING_INSTANCE)
        desc.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    for (u32 i = 0; i < numMembers; i++) {
        SEStructSpec member = layout[i];

        VkVertexInputAttributeDescription vertAttr = {
            .binding = binding,
            .location = i,
            .offset = member.offset,
        };

        switch (member.type) {
            case SE_VAR_TYPE_U32: 
                vertAttr.format = VK_FORMAT_R32_UINT;
                break;
            case SE_VAR_TYPE_F32: 
                vertAttr.format = VK_FORMAT_R32_SFLOAT;
                break;
            case SE_VAR_TYPE_V2F: 
                vertAttr.format = VK_FORMAT_R32G32_SFLOAT;
                break;
            case SE_VAR_TYPE_V3F: 
                vertAttr.format = VK_FORMAT_R32G32B32_SFLOAT;
                break;
            default: todo();
        }

        dynPush(rinfo->attrs, vertAttr);
        dynBack(rinfo->passes).vertInfo.numAttrs++;
        size += member.size;
    }

    desc.stride = size;
    dynPush(rinfo->bindings, desc);

}
