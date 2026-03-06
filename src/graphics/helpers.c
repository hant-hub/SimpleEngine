#include "se.h"
#include "vulkan/vulkan_core.h"
#include <graphics/graphics_intern.h>

VkPipelineLayout CreatePipelineLayout(SEVulkan* v) {

    VkPipelineLayoutCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    };

    VkPipelineLayout layout = 0;
    vkCreatePipelineLayout(v->dev, &info, NULL, &layout);
    return layout;
}

VkPipeline CreatePipeline(SEVulkan *v, VkRenderPass r, PipelineInfo *info) {

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
    info->layout = CreatePipelineLayout(v);

    VkGraphicsPipelineCreateInfo pipeInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .layout = info->layout,
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
