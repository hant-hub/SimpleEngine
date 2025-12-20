#include "cutils.h"
#include "se.h"
#include "vulkan/vulkan_core.h"
#include <graphics/graphics_intern.h>

void BeginRender(SEwindow* win) {
    SEVulkan* v = GetGraphics(win);

    vkWaitForFences(v->dev, 1, &v->inFlight, VK_TRUE, UINT32_MAX);
    vkResetFences(v->dev, 1, &v->inFlight);

    vkResetCommandPool(v->dev, v->pool, 0);

    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandBufferCount = 1,
        .commandPool = v->pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    };

    vkAllocateCommandBuffers(v->dev, &allocInfo, &v->buf);

    VkCommandBufferBeginInfo begInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };

    vkBeginCommandBuffer(v->buf, &begInfo);

}

void EndRender(SEwindow* win, u32 idx) {
    SEVulkan* v = GetGraphics(win);
    vkEndCommandBuffer(v->buf);

    VkSubmitInfo subInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &v->buf,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &v->imgAvalible,
        .pWaitDstStageMask = (VkPipelineStageFlags[]){VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &v->renderfinished,
    };

    vkQueueSubmit(v->queues.graphics, 1, &subInfo, v->inFlight);


    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &v->renderfinished,
        .swapchainCount = 1,
        .pSwapchains = &v->swapchain.swap,
        .pImageIndices = &idx,
    };

    vkQueuePresentKHR(v->queues.present, &presentInfo);
}

u32 BeginRenderPass(SEwindow* win, SERenderPass* r) {
    SEVulkan* v = GetGraphics(win);
    u32 i = 0;
    
    vkAcquireNextImageKHR(v->dev, v->swapchain.swap, UINT32_MAX, v->imgAvalible, NULL, &i);

    VkClearValue clearValue = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

    VkRenderPassBeginInfo passInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = r->pass,
        .framebuffer = r->buffers[i],
        .renderArea = {
            .offset = {0, 0},
            .extent = {v->swapchain.width, v->swapchain.height},
        },
        .clearValueCount = 1,
        .pClearValues = &clearValue,
    };

    vkCmdBeginRenderPass(v->buf, &passInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(v->buf, VK_PIPELINE_BIND_POINT_GRAPHICS, v->resources.pipelines.slots[r->pipeline]);


    VkViewport view = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)v->swapchain.width,
        .height = (float)v->swapchain.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    vkCmdSetViewport(v->buf, 0, 1, &view);

    VkRect2D scissor = {
        .offset = {0.0f, 0.0f},
        .extent = {(float)v->swapchain.width, (float)v->swapchain.height},
    };
    vkCmdSetScissor(v->buf, 0, 1, &scissor);
    return i;
}

void EndRenderPass(SEwindow* win) {
    SEVulkan* v = GetGraphics(win);
    vkCmdEndRenderPass(v->buf);
}

void DrawTriangle(SEwindow* win) {
    SEVulkan* v = GetGraphics(win);
    vkCmdDraw(v->buf, 3, 1, 0, 0);
}
