#include "cutils.h"
#include "se.h"
#include "vulkan/vulkan_core.h"
#include <graphics/graphics_intern.h>


//in future, pass in a vertex struct, and
//uniform size. Then we construct descriptor
//sets and stuff for that.
//TODO(ELI): Far future, check shader reflection for this
u32 SEAddLayout(SEwindow* win) {
    SEVulkan* v = GetGraphics(win);

    u32 layout = LayoutPoolAlloc(&v->resources.layouts);

    
    VkPipelineLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    };

    vkCreatePipelineLayout(v->dev, &layoutInfo, NULL, &v->resources.layouts.slots[layout]);
    return layout;
}

u32 SEAddPipeline(SEwindow* win, SERenderPass* r, u32 layout, u32 vert, u32 frag) {
    SEVulkan* v = GetGraphics(win);

    VkPipelineShaderStageCreateInfo shaders[2]; 
    shaders[0] = (VkPipelineShaderStageCreateInfo){
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .pName = "main",
        .module = v->resources.shaders.slots[vert],
    };
    shaders[1] = (VkPipelineShaderStageCreateInfo){
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pName = "main",
        .module = v->resources.shaders.slots[frag],
    };

    VkPipelineVertexInputStateCreateInfo vertinput = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexAttributeDescriptionCount = 0,
        .vertexBindingDescriptionCount = 0,
    };

    VkPipelineInputAssemblyStateCreateInfo assem = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    VkPipelineViewportStateCreateInfo view = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };

    VkPipelineDynamicStateCreateInfo dyn = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates = (VkDynamicState[]){
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        },
    };

    VkPipelineRasterizationStateCreateInfo raster = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
    };

    VkPipelineMultisampleStateCreateInfo multi = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    };

    VkPipelineColorBlendAttachmentState blend = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                          VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT |
                          VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE,
    };

    VkPipelineColorBlendStateCreateInfo color = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &blend,
        .logicOpEnable = VK_FALSE,
    };


    VkGraphicsPipelineCreateInfo pipeInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .layout = v->resources.layouts.slots[layout],
        .subpass = 0,
        .pColorBlendState = &color,
        .pVertexInputState = &vertinput,
        .pInputAssemblyState = &assem,
        .pDynamicState = &dyn,
        .pRasterizationState = &raster,
        .pMultisampleState = &multi,
        .pViewportState = &view,
        .stageCount = 2,
        .pStages = shaders,
        .renderPass = r->pass,
    };

    u32 pipe = PipelinePoolAlloc(&v->resources.pipelines);
    vkCreateGraphicsPipelines(v->dev, NULL, 1, &pipeInfo, NULL, &v->resources.pipelines.slots[pipe]);

    r->pipeline = pipe;

    return pipe;
}


//for now allocate directly, in future
//create a pool on the window which can
//be destroyed all at once
SERenderPass* SECreateRenderPass(SEwindow* win) {
    SEVulkan* v = GetGraphics(win);
    SERenderPass* r = Alloc(win->mem, sizeof(SERenderPass));

    VkAttachmentDescription adescrip = {
        .format = v->swapchain.format.format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,

        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,

        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentReference aref = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDependency subdep = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };

    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &aref,
    };

    VkRenderPassCreateInfo rpass = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pSubpasses = &subpass,
        .subpassCount = 1,
        .attachmentCount = 1,
        .pAttachments = &adescrip,
        .dependencyCount = 1,
        .pDependencies = &subdep,
    };

    vkCreateRenderPass(v->dev, &rpass, NULL, &r->pass);

    r->numBuffers = v->swapchain.imgcount;
    r->buffers = Alloc(win->mem, r->numBuffers * sizeof(VkFramebuffer));

    for (u32 i = 0; i < v->swapchain.imgcount; i++) {
        VkImageView swapview = v->swapchain.views[i];

        VkFramebufferCreateInfo frameInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = &swapview,
            .height = v->swapchain.height,
            .width = v->swapchain.width,
            .layers = 1,
            .renderPass = r->pass,
        };

        vkCreateFramebuffer(v->dev, &frameInfo, NULL, &r->buffers[i]);
    }

    return r;
}

void SEDestroyRenderPass(SEwindow* win, SERenderPass* r) {
    SEVulkan* v = GetGraphics(win);
    vkDeviceWaitIdle(v->dev);

    for (u32 i = 0; i < r->numBuffers; i++) {
        vkDestroyFramebuffer(v->dev, r->buffers[i], NULL);
    }

    vkDestroyRenderPass(v->dev, r->pass, NULL);
    Free(win->mem, r->buffers, r->numBuffers * sizeof(VkFramebuffer));
    Free(win->mem, r, sizeof(SERenderPass));
}

