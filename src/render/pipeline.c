#include "render/utils.h"
#include "render/vertex.h"
#include "util.h"
#include <platform.h>
#include <render/render.h>
#include <render/pipeline.h>
#include <render/memory.h>
#include <vulkan/vulkan_core.h>



SE_shaders SE_LoadShaders(SE_render_context* r, const char* vert, const char* frag) {
    SE_shaders s;

    SE_string vbuf = SE_LoadFile(vert);
    SE_string fbuf = SE_LoadFile(frag);

    VkShaderModuleCreateInfo vmodule = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pCode = (u32*)vbuf.data,
        .codeSize = vbuf.size
    };

    VkShaderModuleCreateInfo fmodule = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pCode = (u32*)fbuf.data,
        .codeSize = fbuf.size
    };

    REQUIRE_ZERO(vkCreateShaderModule(r->l, &vmodule, NULL, &s.vert));
    REQUIRE_ZERO(vkCreateShaderModule(r->l, &fmodule, NULL, &s.frag));

    SE_UnloadFile(&vbuf);
    SE_UnloadFile(&fbuf);

    SE_Log("Shaders Loaded: %s %s\n", vert, frag);
    return s;
}

static VkImageCreateInfo img_presets[] = {
    //SE_DEPTH_ATTACHMENT ----------------------------
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .samples = VK_SAMPLE_COUNT_1_BIT, //NOTE(ELI): Will need to change based on Multisampling
        .arrayLayers = 1,
        .mipLevels = 1,
    },
};

static VkImageViewCreateInfo view_presets[] = {
    //SE_DEPTH_ATTACHMENT -----------------------------
    (VkImageViewCreateInfo){
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .subresourceRange = {
            .baseMipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1,
            .levelCount = 1,
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
        },
    },
};


typedef struct Attachment_info {
    VkAttachmentDescription descrip;
    VkAttachmentReference ref;
} Attachment_info;

static const VkFormat depthFormats[] = {
    VK_FORMAT_D32_SFLOAT,
    VK_FORMAT_D32_SFLOAT_S8_UINT,
    VK_FORMAT_D24_UNORM_S8_UINT,
};

static Attachment_info attachinfos[] = {
    //SWAPCHAIN IMG -----------------------------
    {
        .descrip = {
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        },
        .ref = {
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        },
    },
    //DEPTH ATTACHMENT ---------------------------------
    {
        .descrip = {
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        },
        .ref = {
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        },
    },
    
};

//TODO(ELI): Go and implement a proper dynamic array utility, the current
//Arenas are not quite adequate yet.
SE_attachments SE_CreateAttachments(SE_mem_arena* a, SE_render_context* r, SE_attachment_request* reqs, u32 reqnum) {
    SE_attachments at = {0};

    //subtract off the output image
    at.num = reqnum;

    at.img = SE_ArenaAlloc(a, sizeof(VkImage) * at.num); 
    at.view = SE_ArenaAlloc(a, sizeof(VkImageView) * at.num);

    at.backingmem = SE_CreateResourceTrackerRaw(r, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, MB(2));


    /* TODO(ELI): Expand reference to include sufficient information
     * to create attachment
     *
     * Alternatively, Combine both attachment reference creation and atachment
     * image/view creation into a single function. That way all the information
     * is in a single place.
     *
     * If This is done, the Create Refs function will need to be modified to take
     * in a list of configurations for each attachment, ie: enums which specify the
     * type of attachment, with maybe some access flags to determine when
     * it gets cleared and how
     */

    //NOTE(ELI): Index should be attachment type minus one.
    //This is because SWAPCHAIN IMG does not need to be created
    //since it is tied up with the swapchain.
    for (int i = 1; i < at.num; i++) {
        VkImageCreateInfo imgInfo = img_presets[reqs[i].t - 1];
        VkImageViewCreateInfo viewInfo = view_presets[reqs[i].t - 1];
        switch (reqs[i].t) {
            case SE_DEPTH_ATTACHMENT:
                {
                    imgInfo.extent = (VkExtent3D){r->s.size.width, r->s.size.height, 1};               
                    imgInfo.format = SE_FindSupportedFormat(r, depthFormats,
                            ASIZE(depthFormats), VK_IMAGE_TILING_OPTIMAL,
                            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
                    );

                    viewInfo.format = imgInfo.format;
                } break;
            case SE_COLOR_ATTACHMENT:
            case SE_SWAPCHAIN_IMG: continue;

            default: 
                {
                    SE_Log("Attachment Not Supported\n");
                    continue;
                }
        }
        REQUIRE_ZERO(vkCreateImage(r->l, &imgInfo, NULL, &at.img[i]));
        viewInfo.image = at.img[i];

        VkMemoryRequirements memreq;
        vkGetImageMemoryRequirements(r->l, at.img[i], &memreq);

        SE_Log("Image Bound with: %d %d %.8b\n", memreq.alignment, memreq.size, memreq.memoryTypeBits);
        u64 offset = SE_CreateRaw(&at.backingmem, memreq.size);
        vkBindImageMemory(r->l, at.img[i], at.backingmem.devMem, offset);

        REQUIRE_ZERO(vkCreateImageView(r->l, &viewInfo, NULL, &at.view[i]));
    }

    return at;
}

SE_attachment_refs SE_CreateAttachmentRefs(SE_mem_arena* a, SE_render_context* r, SE_attachment_request* reqs, u32 reqnum) {
    //initialize final color pass
    SE_attachment_refs refs = {0};

    //baseline one output image
    refs.num = reqnum;
    if (!refs.ref) {
        refs.ref = SE_ArenaAlloc(a, sizeof(VkAttachmentReference) * refs.num);
        refs.descrip = SE_ArenaAlloc(a, sizeof(VkAttachmentDescription) * refs.num);
    }

    //final color pass
    for (u32 i = 0; i < reqnum; i++) {

        refs.descrip[i] = attachinfos[reqs[i].t].descrip;
        refs.ref[i] = attachinfos[reqs[i].t].ref;


        switch (reqs[i].t) {
            case SE_SWAPCHAIN_IMG:
                {
                    refs.descrip[i].format = r->s.format.format;
                    refs.ref[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                } break;
            case SE_DEPTH_ATTACHMENT:
                {
                    refs.descrip[i].format = SE_FindSupportedFormat(r, depthFormats,
                            ASIZE(depthFormats), VK_IMAGE_TILING_OPTIMAL,
                            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
                    );
                    SE_Log("format: %d\n", refs.descrip[i].format);

                    refs.ref[i].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                    refs.ref[i].attachment = i;
                } break;
            default:
                {
                    SE_Log("Unknown Attachment Type!\n");
                }
        }
    }


    //TODO(ELI): Finish Attachment setup


    return refs;
}


//NOTE(ELI): The Plan is to Pass in a bunch of ENUM commands for subpasses,
//then convert them one by one into subpasses with correct dependencies and
//then create a render pass

/* 
 * 
 * TODO(ELI): Remember to Add support for custom subpasses, so that users
 * can do lower level graphics operations. Ideally have a table that could
 * be added to or subtracted from via some metaprogramming magic
 *
 */
typedef struct SubpassDef {
    VkSubpassDependency dep;
    VkSubpassDescription des;
} SubpassDef;

//This table is constant and should never change
static SubpassDef subpassdefs[] = {
    //SE_NO_DEPTH_OPAQUE ------------------------------------
    {
        .dep = {
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        },
        .des = {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
        },
    },
    //SE_OPAQUE_PASS --------------------------------------
    {
        .dep = {
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        },
        .des = {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
        },
    },
};


SE_render_pass SE_CreateRenderPass(SE_render_context* r, SE_render_pipeline* p, SE_pass_type* t, u32 num_passes) {
    SE_render_pass pass = {0};

    VkSubpassDependency subDeps[10] = {0};
    VkSubpassDescription subDescrip[10] = {0};

    VkRenderPassCreateInfo rInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pAttachments = p->refs.descrip,
        .pSubpasses = subDescrip,
        .subpassCount = num_passes,
        .pDependencies = subDeps,
        .dependencyCount = num_passes,
    };

    for (u32 i = 0; i < num_passes; i++) {
        subDeps[i] = subpassdefs[t[i]].dep;
        subDescrip[i] = subpassdefs[t[i]].des;

        subDeps[i].dstSubpass = i;
        switch (t[i]) {
            case SE_NO_DEPTH_OPAQUE_PASS:
                {
                    subDescrip[i].pColorAttachments = &p->refs.ref[0];
                } break;
            case SE_OPAQUE_PASS:
                {
                    subDescrip[i].pColorAttachments = &p->refs.ref[0];
                    subDescrip[i].pDepthStencilAttachment = &p->refs.ref[1];
                } break;

        }

        if (i == 0) {
            subDeps[i].srcSubpass = VK_SUBPASS_EXTERNAL;
        }
    }

    rInfo.attachmentCount = 1;


    REQUIRE_ZERO(vkCreateRenderPass(r->l, &rInfo, NULL, &pass.rp));

    return pass;
}

//TODO(ELI): The final version of this is to have a setup where a pipeline
//config is passed in, and then the pipeline is created based on that, some
//kind of array of enums combined with methods to query and allocate attachments
//while creating frame buffers is also necessary.

//TODO(ELI): Set up descriptor set creation, required for input attachments
//which are necessary for things like tonemapping and post processing effects,
//or at least very convienient for.
SE_render_pipeline SE_CreatePipeline(SE_mem_arena a, SE_render_context* r, SE_vertex_spec* vspec, SE_shaders* s) {
    SE_render_pipeline p = {0};
    p.mem = a;

    //TODO(ELI): Set up system for specifying attachments 
    //Perhaps based on the subpass requirements 

    SE_pass_type passes[] = {
        SE_NO_DEPTH_OPAQUE_PASS,
    };
    
    SE_attachment_request requests[] = {
        (SE_attachment_request){SE_SWAPCHAIN_IMG},
        //(SE_attachment_request){SE_DEPTH_ATTACHMENT},
    };

    p.attachments = SE_CreateAttachments(&p.mem, r, requests, 1);
    p.refs = SE_CreateAttachmentRefs(&p.mem, r, requests, 1);

    p.rpasses = SE_HeapAlloc(sizeof(VkRenderPass));
    p.rpasses[0] = SE_CreateRenderPass(r, &p, passes, 1);

    VkPipelineInputAssemblyStateCreateInfo inputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    VkPipelineRasterizationStateCreateInfo rasterInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
    };

    //TODO(ELI): This will need to be constructed based on
    //the shaders used

    VkPipelineDepthStencilStateCreateInfo depthInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 1.0f,
    };

    VkDynamicState dynStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pDynamicStates = dynStates,
        .dynamicStateCount = ASIZE(dynStates)
    };

    VkPipelineViewportStateCreateInfo viewInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };

    VkPipelineColorBlendAttachmentState colorattachInfo = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
    };

    VkPipelineColorBlendStateCreateInfo colorblendInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorattachInfo,
        .logicOp = VK_LOGIC_OP_COPY,
        .logicOpEnable = VK_FALSE,
        .blendConstants = {
           0, 0, 0, 0 
        },
    };

    VkPipelineMultisampleStateCreateInfo multiInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .minSampleShading = 1.0f,
        .pSampleMask = NULL,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };
    

    VkPipelineShaderStageCreateInfo ShaderStages[2];
    ShaderStages[0] = (VkPipelineShaderStageCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .module = s->vert,
        .pSpecializationInfo = NULL,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .pName = "main",
    };
    ShaderStages[1] = (VkPipelineShaderStageCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .module = s->frag,
        .pSpecializationInfo = NULL,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pName = "main",
    };

    //TODO(ELI): In future, move this to shader loading
    //TODO(ELI): Write parser to do basic reflection
    //TODO(ELI): Auto generate descriptor sets for shaders
    VkPipelineLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,

        //descriptor sets ----
        .setLayoutCount = 0,
        .pSetLayouts = NULL,
        //--------------------
        
        .pushConstantRangeCount = 0,
    };

    REQUIRE_ZERO(vkCreatePipelineLayout(r->l, &layoutInfo, NULL, &s->layout));
    VkGraphicsPipelineCreateInfo pipeInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .layout = s->layout,
        .renderPass = p.rpasses[0].rp,
        .subpass = 0,
        .pStages = ShaderStages,
        .stageCount = 2,
        .pInputAssemblyState = &inputInfo,
        .pRasterizationState = &rasterInfo,
        .pVertexInputState = &vspec->inputstate,
        .pDynamicState = &dynInfo,
        .pViewportState = &viewInfo,
        .pColorBlendState = &colorblendInfo,
        .pTessellationState = NULL,
        .pDepthStencilState = &depthInfo,
        .pMultisampleState = &multiInfo,
    };

    p.pipelines = SE_HeapAlloc(sizeof(VkPipeline));
    REQUIRE_ZERO(vkCreateGraphicsPipelines(r->l, VK_NULL_HANDLE, 1, &pipeInfo, NULL, p.pipelines));


    //framebuffers
    SE_CreateFrameBuffers(r, &p);

    //Sync Objects

    SE_Log("Graphics Pipeline and Renderpass Created\n");
    SE_Log("FrameBuffers Created\n");

    return p;
}

void SE_CreateFrameBuffers(SE_render_context* r, SE_render_pipeline* p) {
    p->numframebuffers = r->s.numImgs;
    p->framebuffers = SE_HeapAlloc(sizeof(VkFramebuffer) * r->s.numImgs);
    for (u32 i = 0; i < r->s.numImgs; i++) {
        VkImageView views[] = {
            r->s.views[i],
            p->attachments.view[1]
        };
        VkFramebufferCreateInfo frameInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = views,
            .renderPass = p->rpasses[0].rp,
            .height = r->s.size.height,
            .width = r->s.size.width,
            .layers = 1,
        };

        REQUIRE_ZERO(vkCreateFramebuffer(r->l, &frameInfo, NULL, &p->framebuffers[i]));
    }
}

SE_sync_objs SE_CreateSyncObjs(SE_render_context* r) {
    SE_sync_objs s = {0};
    //TODO(ELI): allow configuration in future
    const u32 numFrames = 1;
    s.numFrames = numFrames;
    s.avalible = SE_HeapAlloc(sizeof(VkSemaphore) * numFrames);  
    s.finished = SE_HeapAlloc(sizeof(VkSemaphore) * numFrames);  
    s.pending = SE_HeapAlloc(sizeof(VkFence) * numFrames);  

    for (u32 i = 0; i < numFrames; i++) {
        VkSemaphoreCreateInfo semInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };

        VkFenceCreateInfo fenceInfo = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT,
        };

        REQUIRE_ZERO(vkCreateSemaphore(r->l, &semInfo, NULL, &s.avalible[i]));
        REQUIRE_ZERO(vkCreateSemaphore(r->l, &semInfo, NULL, &s.finished[i]));
        REQUIRE_ZERO(vkCreateFence(r->l, &fenceInfo, NULL, &s.pending[i]));
    }
    return s;
}

void SE_DestroySyncObjs(SE_render_context* r, SE_sync_objs* s) {
    for (u32 i = 0; i < s->numFrames; i++) {
        vkDestroySemaphore(r->l, s->finished[i], NULL);
        vkDestroySemaphore(r->l, s->avalible[i], NULL);
        vkDestroyFence(r->l, s->pending[i], NULL);
    }
    SE_HeapFree(s->pending);
    SE_HeapFree(s->avalible);
    SE_HeapFree(s->finished);
}

//temporary
void SE_DrawFrame(SE_window* win, SE_render_context* r, SE_render_pipeline* p, SE_sync_objs* s, SE_resource_arena* vert) {

        static u32 frame = 0;
        vkWaitForFences(r->l, 1, &s->pending[frame], VK_TRUE, UINT64_MAX);

        u32 imgIndex;
        VkResult res = vkAcquireNextImageKHR(r->l, r->s.swap, UINT64_MAX, s->avalible[frame], VK_NULL_HANDLE, &imgIndex);
        
        if (/*win->resize ||*/ res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
            vkDeviceWaitIdle(r->l);
            SE_DestroySyncObjs(r, s); 
            *s = SE_CreateSyncObjs(r);
            r->s = SE_CreateSwapChain(NULL, r, win, &r->s);

            //Currently is a memory leak lmao
            //TODO(ELI): Recreate attachments with appropriate size
            //SE_CreateFrameBuffers(r, p);

            win->resize = FALSE;
            return;
        } else if (res) {
            SE_Exit(-1);
        }

        vkResetFences(r->l, 1, &s->pending[frame]);
        
        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };

        REQUIRE_ZERO(vkBeginCommandBuffer(r->cmd, &beginInfo));

        VkClearValue clearcolors[] = {
            {{0.0f, 0.0f, 0.0f, 0.0f}},
            (VkClearValue){
                .depthStencil.depth = 1.0f
            },
        };

        VkRenderPassBeginInfo renderpassInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = p->rpasses[0].rp,
            .framebuffer = p->framebuffers[imgIndex],
            .renderArea = {
                .extent = r->s.size,
                .offset = {0, 0}
            },
            .clearValueCount = 2,
            .pClearValues = clearcolors
        };

        vkCmdBeginRenderPass(r->cmd, &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(r->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, p->pipelines[0]);

        VkViewport view = {
            .x = 0,
            .y = 0,
            .width = r->s.size.width,
            .height = r->s.size.height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        vkCmdSetViewport(r->cmd, 0, 1, &view);

        VkRect2D scissor = {
            .offset = {0, 0},
            .extent = r->s.size
        };
        vkCmdSetScissor(r->cmd, 0, 1, &scissor);

        
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(r->cmd, 0, 1, (VkBuffer*)(&vert->resource), offsets);

        vkCmdDraw(r->cmd, 6, 1, 0, 0);
        vkCmdEndRenderPass(r->cmd);
        REQUIRE_ZERO(vkEndCommandBuffer(r->cmd));
        

        const VkPipelineStageFlags waitStage = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}; 
        VkSubmitInfo subInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &r->cmd,
            .pWaitSemaphores = &s->avalible[frame],
            .waitSemaphoreCount = 1,
            .pSignalSemaphores = &s->finished[frame],
            .signalSemaphoreCount = 1,
            .pWaitDstStageMask = &waitStage,
        };

        REQUIRE_ZERO(vkQueueSubmit(r->Queues.g, 1, &subInfo, s->pending[frame]));
        
        VkPresentInfoKHR presentInfo = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &s->finished[frame],
            .swapchainCount = 1,
            .pSwapchains = &r->s.swap,
            .pImageIndices = &imgIndex,
            .pResults = NULL,
        };

        REQUIRE_ZERO(vkQueuePresentKHR(r->Queues.p, &presentInfo));
}
