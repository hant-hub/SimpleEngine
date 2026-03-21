#include "se.h"
#include "vulkan/vulkan_core.h"
#include <graphics/graphics_intern.h>

VkPipelineLayout CreatePipelineLayout(SEVulkan* v, DescriptorLayout* l) {

    l->info.pBindings = l->bindings.data;
    l->info.bindingCount = l->bindings.size;

    vkCreateDescriptorSetLayout(v->dev, &l->info, NULL, &l->desclayout);

    VkPipelineLayoutCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pSetLayouts = &l->desclayout,
        .setLayoutCount = 1,
    };

    VkPipelineLayout layout = 0;
    vkCreatePipelineLayout(v->dev, &info, NULL, &layout);
    return layout;
}

VkPipeline CreatePipeline(SEVulkan *v, VkRenderPass r, PipelineInfo *info, VkPipelineLayout layout) {

    VkPipelineShaderStageCreateInfo shaders[2];
    shaders[0] = (VkPipelineShaderStageCreateInfo){
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .pName = "main",
        .module = info->vert,
    };
    shaders[1] = (VkPipelineShaderStageCreateInfo){
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pName = "main",
        .module = info->frag,
    };

    VkPipelineVertexInputStateCreateInfo vertinput = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexAttributeDescriptionCount = info->attrs.size,
        .vertexBindingDescriptionCount = info->bindings.size,
        .pVertexAttributeDescriptions = info->attrs.data,
        .pVertexBindingDescriptions = info->bindings.data,
    };

    //TODO(ELI): Extract later

    VkGraphicsPipelineCreateInfo pipeInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .layout = layout,
        .subpass = 0,
        .pColorBlendState = info->pColorBlendState.sType ? &info->pColorBlendState : NULL,
        .pVertexInputState = &vertinput,
        .pInputAssemblyState = info->pInputAssemblyState.sType ? &info->pInputAssemblyState : NULL,
        .pDynamicState = info->pDynamicState.sType ? &info->pDynamicState : NULL,
        .pRasterizationState = info->pRasterizationState.sType ? &info->pRasterizationState : NULL,
        .pMultisampleState = info->pMultisampleState.sType ? &info->pMultisampleState : NULL,
        .pViewportState = info->pViewportState.sType ? &info->pViewportState : NULL,
        .stageCount = 2,
        .pStages = shaders,
        .renderPass = r,
    };

    VkPipeline pipe = 0;
    vkCreateGraphicsPipelines(v->dev, NULL, 1, &pipeInfo, NULL, &pipe);

    return pipe;
}

VkShaderModule CompileShader(SEVulkan* v, SString data) {
    VkShaderModuleCreateInfo moduleInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = data.len,
        .pCode = (u32*)data.data,
    };

    VkShaderModule m = 0;
    vkCreateShaderModule(v->dev, &moduleInfo, NULL, &m);
    return m;
}

Layout CompileLayout(SEVulkan* v, DescriptorLayout* layout) {
    Layout l;
    ScratchArena sc = ScratchArenaGet(NULL);
    
    VkDescriptorPoolSize* sizes = ArenaAllocZero(&sc.arena, sizeof(VkDescriptorPoolSize) * layout->bindings.size); 
    for (u32 i = 0; i < layout->bindings.size; i++) {
        sizes[i] = (VkDescriptorPoolSize) {
            .type = layout->bindings.data[i].descriptorType,
            .descriptorCount = layout->bindings.data[i].descriptorCount,
        };
    }

    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pPoolSizes = sizes,
        .poolSizeCount = layout->bindings.size,
        .maxSets = layout->sets_required,
    };

    vkCreateDescriptorPool(v->dev, &poolInfo, NULL, &l.pool);
    ScratchArenaEnd(sc);
    return l;
}
