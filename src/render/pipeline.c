#include <stdint.h>
#include <assert.h>

#include <util.h>
#include <internal.h>
#include <platform.h>
#include <render/cache.h>
#include <render/utils.h>
#include <render/vertex.h>
#include <render/render.h>
#include <render/pipeline.h>
#include <render/memory.h>
#include <vulkan/vulkan_core.h>


SE_LoadShaderFunc(SE_LoadShaders) {
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

    VkPipelineLayoutCreateInfo layoutinfo = (VkPipelineLayoutCreateInfo){
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pushConstantRangeCount = 0,
    };
    REQUIRE_ZERO(vkCreatePipelineLayout(r->l, &layoutinfo, NULL, &s.layout));

    SE_Log("Shaders Loaded: %s %s\n", vert, frag);
    return s;
}


SE_CreateSyncObjsFunc(SE_CreateSyncObjs) {
    VkRenderPassCreateInfo i;

    SE_sync_objs s = {0};
    //TODO(ELI): allow configuration in future
    const u32 numFrames = r->s.numImgs;
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

//idk man maybe add stuff
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

//Helper funcs for Pipeline -----------------------------------

void SE_push_attach_ref(SE_render_pipeline_info* p, SE_subpass_attach s) {
    if (p->rsize + 1 > p->rcap) {
        p->rcap = p->rcap ? p->rcap * 2 : 2;
        p->attach_refs = SE_HeapRealloc(p->attach_refs, sizeof(SE_subpass_attach) * p->rcap);
    }
    p->attach_refs[p->rsize++] = s;
}

void SE_push_pass(SE_render_pipeline_info* p, SE_sub_pass s) {
    if (p->psize + 1 > p->pcap) {
        p->pcap = p->pcap ? p->pcap * 2 : 2;
        p->passes = SE_HeapRealloc(p->passes, sizeof(SE_sub_pass) * p->pcap);
    }
    p->passes[p->psize++] = s;
}

void SE_push_attachment(SE_render_pipeline_info* p, SE_attachment_type t) {
    if (p->atsize + 1 > p->atcap) {
        p->atcap = p->atcap ? p->atcap * 2 : 2;
        p->attachments = SE_HeapRealloc(p->attachments, sizeof(SE_attachment_type) * p->atcap);
    }
    p->attachments[p->atsize++] = t;
}

void SE_push_shaders(SE_render_pipeline_info* p, SE_shaders s) {
    if (p->ssize + 1 > p->scap) {
        p->scap = p->scap ? p->scap * 2 : 2;
        p->shaders = SE_HeapRealloc(p->shaders, sizeof(SE_shaders) * p->scap);
    }
    p->shaders[p->ssize++] = s;
}

void SE_push_vert(SE_render_pipeline_info* p, SE_vertex_spec v) {
    if (p->vsize + 1 > p->vcap) {
        p->vcap = p->vcap ? p->vcap * 2 : 2;
        p->verts = SE_HeapRealloc(p->verts, sizeof(SE_vertex_spec) * p->vcap);
    }
    p->verts[p->vsize++] = v;
}

//-------------------------------------------------------------

/*
 *  The final pass will use index zero always
 *
 */
SE_BeginPipelineCreationFunc(SE_BeginPipelineCreation) {
    SE_render_pipeline_info info = (SE_render_pipeline_info){0};
    SE_push_attachment(&info, SE_SwapImg);

    return (SE_render_pipeline){
        .pipelineInfo = info
    };
}

SE_AddShaderFunc(SE_AddShader) {
    SE_render_pipeline_info* p = &pipe->pipelineInfo;
    u32 output = p->ssize;
    s->pipelineIdx = pipeline;
    SE_push_shaders(p, *s); 
    return output;
}

SE_AddVertSpecFunc(SE_AddVertSpec) {
    SE_render_pipeline_info* p = &pipe->pipelineInfo;
    u32 output = p->vsize;
    SE_push_vert(p, *v); 
    return output;
}

//TODO(ELI): Functions for making each attachment type

SE_AddDepthAttachmentFunc(SE_AddDepthAttachment) {
    SE_render_pipeline_info* p = &pipe->pipelineInfo;
    SE_push_attachment(p, SE_DepthImg);
    return p->atsize - 1;
}

u32 SE_AddStencilAttachment(SE_render_pipeline_info* p) {
    SE_Log("Not Implemented\n");
    assert(0);
    return -1;
}

//Generally depth and stencil attachments have to be combined, its kinda lame
//but it makes a lot of sense, a stencil is just an attachment with maximum depth
//and minimum depths which either prevent or allow rendering
u32 SE_AddDepthAndStencilAttachment(SE_render_pipeline_info* p) {
    SE_Log("Not Implemented\n");
    assert(0);
    return -1;
}

u32 SE_AddColorAttachment(SE_render_pipeline_info* p) {
    SE_Log("Not Implemented\n");
    assert(0);
    return -1;
}

//General purpose attachment, essentially a fancy buffer
u32 SE_AddInputAttachment(SE_render_pipeline_info* p) {
    SE_Log("Not Implemented\n");
    assert(0);
    return -1;
}

SE_OpqaueNoDepthPassFunc(SE_OpqaueNoDepthPass) {
    SE_render_pipeline_info* p = &pipe->pipelineInfo;
    SE_subpass_attach a = (SE_subpass_attach) {
        .usage = SE_attach_color,
        .attach_idx = target,
    };

    SE_sub_pass s = (SE_sub_pass) {
        .shader = shader,
        .vert = vert,
        .start = p->rsize,
        .num = 1,
    };

    SE_push_attach_ref(p, a); 
    SE_push_pass(p, s);
}

SE_OpaquePassFunc(SE_OpqauePass) {
    SE_render_pipeline_info* p = &pipe->pipelineInfo;
    SE_subpass_attach a = (SE_subpass_attach) {
        .usage = SE_attach_color,
        .attach_idx = target,
    };

    SE_subpass_attach d = (SE_subpass_attach) {
        .usage = SE_attach_depth,
        .attach_idx = depth,
    };

    SE_sub_pass s = (SE_sub_pass) {
        .shader = shader,
        .vert = vert,
        .start = p->rsize,
        .num = 2,
    };

    SE_push_attach_ref(p, a); 
    SE_push_attach_ref(p, d); 
    SE_push_pass(p, s);
}

void SE_NextRenderPass(SE_render_pipeline_info* p) {
    SE_sub_pass s = (SE_sub_pass) {
        .num = 0
    };
    SE_push_pass(p, s);
}

SE_EndPipelineCreationFunc(SE_EndPipelineCreation) {
    SE_render_pipeline_info* info = &p->pipelineInfo;
    SE_NextRenderPass(info);

    p->numSubpasses = info->psize;
    p->mem.ctx = SE_HeapArenaCreate(KB(10));
    p->mem.alloc = SE_StaticArenaAlloc;

    SE_mem_arena* m = SE_HeapArenaCreate(KB(12));
    SE_allocator a = (SE_allocator) {
        .alloc = SE_StaticArenaAlloc,
        .ctx = m
    };

    p->backingmem = SE_CreateResourceTrackerRaw(r, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, MB(1));


    //make attachments
    // 0 is always the screen


    u32 num_input = 0;
    u32 num_colors = 0;
    u32 num_depth = 0;
    u32 num_resolve = 0;

    u32 num_renderpasses = 1;
    
    //Calculate number of attachment references total
    for (u32 i = 0; i < info->rsize; i++) {
        switch (info->attach_refs[i].usage) {
            case SE_attach_color:
                {
                    num_colors++;
                } break;
            case SE_attach_depth:
                {
                    num_depth++;
                } break;
            case SE_attach_input:
                {
                    num_input++;
                } break;
            default:
                {
                    SE_Log("Invalid Attachment Usage\n");
                    assert(0);
                }
        }
    }



    SE_Log("%d %d %d %d\n", num_colors, num_depth, num_input, num_resolve);


    /*
     * Current plan:
     *
     * One description per reference, That way as we construct each pass we
     * can dynamically choose how to load and store each attachment, as well
     * as transition it.
     *
     * Currently, we should just bake in default behavior to just not perserve
     * anything between passes, but in future add more nuanced behavior.
     *
     * */
    
    VkAttachmentDescription* descrip = a.alloc(0, sizeof(VkAttachmentDescription) * (num_input + num_colors + num_depth + num_resolve), NULL, a.ctx);
    VkAttachmentDescription* rdescrip = a.alloc(0, sizeof(VkAttachmentDescription) * info->atsize, NULL, a.ctx);
    u8* idescrip = a.alloc(0, sizeof(u8) * info->atsize, NULL, a.ctx);
    u8* descrip_flags = a.alloc(0, sizeof(u8) * (num_input + num_colors + num_depth + num_resolve), NULL, a.ctx);
    u32 curr_descrip = 0;

    VkAttachmentReference* input = a.alloc(0, sizeof(VkAttachmentReference) * num_input, NULL, a.ctx);
    VkAttachmentReference* colors = a.alloc(0, sizeof(VkAttachmentReference) * num_colors, NULL, a.ctx);
    VkAttachmentReference* depth = a.alloc(0, sizeof(VkAttachmentReference) * num_depth, NULL, a.ctx);
    VkAttachmentReference* resolve = a.alloc(0, sizeof(VkAttachmentReference) * num_resolve, NULL, a.ctx);

    VkSubpassDescription* subDes = a.alloc(0, sizeof(VkSubpassDescription) * info->psize, NULL, a.ctx);
    VkSubpassDependency* subDep = a.alloc(0, sizeof(VkSubpassDependency) * info->psize, NULL, a.ctx);

    u32 curr_input = 0;
    u32 curr_colors = 0;
    u32 curr_depth = 0;
    u32 curr_resolve = 0;
    u32 numSubpasses = 0;

    p->numPasses = 0;
    for (u32 i = 0; i < info->psize; i++) {
        if (info->passes[i].num == 0) p->numPasses++;
    }
    p->rpasses = p->mem.alloc(0, sizeof(SE_render_pass) * p->numPasses, NULL, p->mem.ctx);

    u32 currRpass = 0;

    for (u32 i = 0; i < info->atsize; i++) {
        switch(info->attachments[i]) {
            case SE_SwapImg:
            case SE_ColorImg:
                {
                    descrip[i] = (VkAttachmentDescription) {
                        .format = r->s.format.format,
                        .samples = VK_SAMPLE_COUNT_1_BIT,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                        .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    };

                    if (info->attachments[i] == SE_SwapImg) {
                        descrip[i].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                    }
                } break;
            case SE_DepthImg:
                {
                    SE_Log("Depth attachment at: %d\n", i);
                    descrip[i] = (VkAttachmentDescription) {
                        .format = VK_FORMAT_D32_SFLOAT,
                        .samples = VK_SAMPLE_COUNT_1_BIT,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                    };

                } break;
            default:
                {
                    SE_Log("Invalid Attachment Usage\n");
                    assert(0);
                }
        }
        descrip_flags[i] = 0;
    }


    //Each step needs to build the description and the
    //reference, then finalize the subpass
    SE_Log("Psize: %d\n", info->psize);
    for (u32 i = 0; i < info->psize; i++) {
        SE_Log("Pass: %d\n", i);
        if (info->passes[i].num == 0) {
            //build renderpasses

            for (u32 j = 0; j < info->atsize; j++) {
                if (descrip_flags[j]) {
                    SE_Log("hit: %d %d\n", curr_descrip, descrip[j].samples);
                    idescrip[j] = curr_descrip;
                    rdescrip[curr_descrip++] = descrip[j];
                }
                descrip_flags[j] = 0;
            }

            for (u32 j = 0; j < num_colors; j++) {
                colors[j].attachment = idescrip[colors[j].attachment];
            }
            for (u32 j = 0; j < num_depth; j++) {
                depth[j].attachment = idescrip[colors[j].attachment];
            }


            SE_Log("Renderpass Attachment Count: %d\n", curr_descrip);

            VkRenderPassCreateInfo rinfo = (VkRenderPassCreateInfo) {
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                    .subpassCount = numSubpasses,
                    .dependencyCount = numSubpasses,
                    .pSubpasses = subDes,
                    .pDependencies = subDep,
                    .attachmentCount = curr_descrip,
                    .pAttachments = rdescrip,
            };

            curr_input = 0;
            curr_colors = 0;
            curr_depth = 0;
            curr_resolve = 0;
            curr_descrip = 0;
            numSubpasses = 0;

            SE_Log("passes: %d\n", info->psize);

            REQUIRE_ZERO(vkCreateRenderPass(r->l, &rinfo, NULL, &p->rpasses[currRpass++].rp));
            continue;
        }
        numSubpasses++;

        u32 start_input = curr_input;
        u32 start_colors = curr_colors;
        u32 start_depth = curr_depth;
        u32 start_resolve = curr_resolve;

        subDep[i] = (VkSubpassDependency){
            .dstSubpass = i,
            .srcSubpass = i == 0 ? VK_SUBPASS_EXTERNAL : i - 1,
        };
        for (u32 j = 0; j < info->passes[i].num; j++) {
            switch(info->attach_refs[j + info->passes[i].start].usage) {
                case SE_attach_color:
                    {
                        //descrip[curr_descrip] = (VkAttachmentDescription) {
                        //    .format = r->s.format.format,
                        //    .samples = VK_SAMPLE_COUNT_1_BIT,
                        //    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                        //    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                        //    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                        //    .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                        //};


                        colors[curr_colors++] = (VkAttachmentReference) {
                            .attachment = info->attach_refs[j + info->passes[i].start].attach_idx,
                            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                        };

                        subDep[i].srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                        subDep[i].dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                        subDep[i].dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

                        descrip_flags[curr_descrip] = 1;
                    } break;
                case SE_attach_depth:
                    {
                        SE_Log("Depth attachment at: %d\n", curr_descrip);
                        //descrip[curr_descrip] = (VkAttachmentDescription) {
                        //    .format = VK_FORMAT_D32_SFLOAT, //TODO(ELI): revisit later
                        //    .samples = VK_SAMPLE_COUNT_1_BIT,
                        //    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                        //    .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                        //    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                        //    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                        //    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                        //    .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                        //};


                        depth[curr_depth++] = (VkAttachmentReference) {
                            .attachment = info->attach_refs[j + info->passes[i].start].attach_idx,
                            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                        };

                        subDep[i].srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
                        subDep[i].dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
                        subDep[i].dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                        descrip_flags[curr_descrip] = 1;
                    } break;
                case SE_attach_input:
                    {
                        SE_Log("Not Implemented\n");
                        assert(0);
                    } break;
                default:
                    {
                        SE_Log("Invalid Attachment Usage\n");
                        assert(0);
                    }

            }
        }

        SE_Log("Total Descriptions: %d\n", curr_descrip);
        SE_Log("Attachments Required:\n");
        SE_Log("\tcolor: %d\n", curr_colors - start_colors);
        SE_Log("\tdepth: %d\n", curr_depth - start_depth);


        
        subDes[i] = (VkSubpassDescription) {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,

            .colorAttachmentCount = curr_colors - start_colors,
            .pColorAttachments = &colors[start_colors],

            .inputAttachmentCount = curr_input - start_input,
            .pInputAttachments = &input[start_input],

            .pResolveAttachments = NULL, //would be the same as color attachments but with multisample buffers
                                         
            //TODO(ELI): Rework later to take advantage of single depth attachment
            .pDepthStencilAttachment = (curr_depth - start_depth) ? &depth[0]: NULL,
        };
    }


    //Build pipelines
    //TODO(ELI): Setup Pipeline Cache file
    //Apparently Base Pipeline is not useful, so don't
    //bother implementing

    //TODO(ELI): Hash pipelines? If there are two renderpasses,
    //and the first subpasses both use the same pipeline,
    //they can share pipelines. Hashing might help ensure
    //that it works

    u32 subp = 0;
    u32 rp = 0;
    Bool8* pipeline_built = a.alloc(0, sizeof(Bool8) * info->ssize, NULL, a.ctx);
    SE_memset(pipeline_built, 0, sizeof(Bool8) * info->ssize);

    for (u32 j = 0; j < info->psize; j++) {
        if (info->passes[j].num == 0) { 
            subp = 0;
            rp++;
            continue;
        }
        u32 shader_idx = info->passes[j].shader;
        u32 i = info->shaders[info->passes[j].shader].pipelineIdx;

        if (pipeline_built[shader_idx]) {
            continue;
        }

        SE_Log("Pipeline Requested:\n");
        SE_Log("\tshaders: %d\n", c->pipelines[i].shaders);
        VkPipelineShaderStageCreateInfo stages[2] = {
            (VkPipelineShaderStageCreateInfo) {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .module = info->shaders[c->pipelines[i].shaders].vert,
                .stage = VK_SHADER_STAGE_VERTEX_BIT,
                .pName = "main",
            },
            (VkPipelineShaderStageCreateInfo) {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .module = info->shaders[c->pipelines[i].shaders].frag,
                .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                .pName = "main",
            },
        };

        VkPipelineInputAssemblyStateCreateInfo assemInfo = (VkPipelineInputAssemblyStateCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = SE_primitive_map[c->pipelines[i].t],
            .primitiveRestartEnable = VK_FALSE,
        };

        VkPipelineRasterizationStateCreateInfo rasterInfo = (VkPipelineRasterizationStateCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .cullMode = SE_cull_mode_map[c->pipelines[i].raster.cull_mode],
            .polygonMode = SE_polygon_mode_map[c->pipelines[i].raster.polymode],

            .depthBiasEnable = c->pipelines[i].raster.depthBias,
            .depthBiasConstantFactor = c->pipelines[i].raster.depthBiasConstantFactor,
            .depthBiasSlopeFactor = c->pipelines[i].raster.depthBiasSlopeFactor,
            .depthBiasClamp = c->pipelines[i].raster.depthBiasClamp,
            .depthClampEnable = c->pipelines[i].raster.depthClamp,

            .rasterizerDiscardEnable = c->pipelines[i].raster.rasterDiscard,
            .frontFace = SE_frontface_map[c->pipelines[i].raster.winding],
            .lineWidth = c->pipelines[i].raster.lineWidth ? c->pipelines[i].raster.lineWidth : 1.0f  //Force non-zero linewidth
        };

        SE_Log("Pipeline idx: %d\n", i);
        SE_Log("Raster Discard Enable: %d\n", c->pipelines[i].raster.rasterDiscard);

        VkPipelineMultisampleStateCreateInfo multiInfo = (VkPipelineMultisampleStateCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .minSampleShading = 1.0f,
        };

        VkPipelineDepthStencilStateCreateInfo depthInfo = (VkPipelineDepthStencilStateCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthBoundsTestEnable = c->pipelines[i].depth.depthTestEnable,
            .depthWriteEnable = c->pipelines[i].depth.depthWriteEnable,
            .depthCompareOp = SE_compare_map[c->pipelines[i].depth.op],
            .stencilTestEnable = VK_FALSE,
        };

        VkPipelineColorBlendAttachmentState blendInfo = (VkPipelineColorBlendAttachmentState) {
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
                .blendEnable = VK_FALSE,
                .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
                .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                .colorBlendOp = VK_BLEND_OP_ADD,
                .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                .alphaBlendOp = VK_BLEND_OP_ADD,
        };

        VkPipelineColorBlendStateCreateInfo colorInfo = (VkPipelineColorBlendStateCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                .logicOp = VK_LOGIC_OP_COPY,
                .logicOpEnable = VK_FALSE,
                .attachmentCount = 1,
                .pAttachments = &blendInfo,
                .blendConstants[0] = 0.0f,
                .blendConstants[1] = 0.0f,
                .blendConstants[2] = 0.0f,
                .blendConstants[3] = 0.0f,
        };

        VkDynamicState states[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };

        VkPipelineDynamicStateCreateInfo dynstate = (VkPipelineDynamicStateCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pDynamicStates = states,
            .dynamicStateCount = ASIZE(states),
        };

        VkPipelineViewportStateCreateInfo viewInfo = (VkPipelineViewportStateCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1,
        };


        VkGraphicsPipelineCreateInfo pipeInfo = (VkGraphicsPipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .basePipelineHandle = NULL,
            .basePipelineIndex = 0,
            .layout = info->shaders[0].layout,
            .stageCount = 2,
            .pStages = stages, 
            .pVertexInputState = &info->verts[0].inputstate,
            .pInputAssemblyState = &assemInfo,
            .pTessellationState = NULL,
            .pViewportState = &viewInfo,
            .pRasterizationState = &rasterInfo,
            .pMultisampleState = &multiInfo,
            .pDepthStencilState = &depthInfo,
            .pColorBlendState = &colorInfo,
            .pDynamicState = &dynstate,

            .renderPass = p->rpasses[rp].rp,
            .subpass = subp++,
        };

        p->pipelines = p->mem.alloc(0, sizeof(VkPipeline), NULL, p->mem.ctx);
        REQUIRE_ZERO(vkCreateGraphicsPipelines(r->l, NULL, 1, &pipeInfo, NULL, &p->pipelines[0]));

        pipeline_built[shader_idx] = TRUE;
    }

    //Build command buffers

    //SE_Log("Command Buffers:\n");
    //SE_Log("\tcount: %d\n", info->psize);

    //VkCommandPoolCreateInfo poolInfo = (VkCommandPoolCreateInfo) {
    //    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    //    .queueFamilyIndex = r->Queues.gfam,
    //};

    //REQUIRE_ZERO(vkCreateCommandPool(r->l, &poolInfo, NULL, &p.pool));

    //p.buffers = p.mem.alloc(0, sizeof(VkCommandBuffer) * info->psize, NULL, p.mem.ctx);
    //VkCommandBufferAllocateInfo cmdInfo = (VkCommandBufferAllocateInfo) {
    //    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    //        .commandPool = p.pool,
    //        .level = VK_COMMAND_BUFFER_LEVEL_SECONDARY,
    //        .commandBufferCount = info->psize,
    //};

    //REQUIRE_ZERO(vkAllocateCommandBuffers(r->l, &cmdInfo, p.buffers));
    //u32 j = 0;
    //for (u32 i = 0; i < info->psize; i++) {
    //    if (info->passes[i].num == 0) {
    //        j++;
    //        continue;
    //    }

    //    VkCommandBufferBeginInfo begInfo = (VkCommandBufferBeginInfo) {
    //        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    //    };

    //}
    
    //TODO(ELI): make this better later
    //Maybe find a better method later? Perhaps combine the config and render pipeline
    //structs and just free the unused arrays?
    p->num_attach = info->atsize;
    p->attach_types = p->mem.alloc(0, sizeof(SE_attachment_type) * p->num_attach, NULL, p->mem.ctx);
    SE_memcpy(p->attach_types, info->attachments, sizeof(SE_attachment_type) * p->num_attach);

    p->imgs = p->mem.alloc(0, sizeof(VkImage) * p->num_attach, NULL, p->mem.ctx);
    p->views = p->mem.alloc(0, sizeof(VkImageView) * p->num_attach, NULL, p->mem.ctx);

    SE_CreateFrameBuffers(r, p);
    p->s = SE_CreateSyncObjs(r);

    SE_HeapFree(m);
}

//Frame Buffer Stuff ------------------------------------------------------------

//TODO(ELI): Make all attachments, maybe consider system for
//fixed resolution attachments
SE_CreateFrameBuffersFunc(SE_CreateFrameBuffers) {

    //make attachments
    SE_Log("Attachments Requested: %d\n", p->num_attach);
    for (u32 i = 0; i < p->num_attach; i++) {
        switch (p->attach_types[i]) {
            case SE_ColorImg:
            {
                SE_Log("Not Implemented\n");
                assert(0);
            } break;
            case SE_DepthImg:
            {
                SE_Log("Depth Attachment: %d\n", r->depthFormat);
                //assert(0); //force crash
                VkImageCreateInfo imgInfo = {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                    .imageType = VK_IMAGE_TYPE_2D,
                    .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                    .samples = VK_SAMPLE_COUNT_1_BIT,
                    .arrayLayers = 1,
                    .extent = {
                        .width = r->s.size.width,
                        .height = r->s.size.height,
                        .depth = 1,
                    },
                    .mipLevels = 1,
                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .tiling = VK_IMAGE_TILING_OPTIMAL,
                    .format = r->depthFormat,
                };

                REQUIRE_ZERO(vkCreateImage(r->l, &imgInfo, NULL, &p->imgs[i]));
                SE_Log("test\n");

                VkMemoryRequirements memreqs;
                vkGetImageMemoryRequirements(r->l, p->imgs[i], &memreqs);
                u64 offset = SE_CreateRaw(&p->backingmem, memreqs.size);  
                REQUIRE_ZERO(vkBindImageMemory(r->l, p->imgs[i], p->backingmem.devMem, offset)); 

                VkImageViewCreateInfo viewInfo = (VkImageViewCreateInfo) {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY,
                    .image = p->imgs[i],
                    .format = r->depthFormat,
                    .components = {
                        .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                        .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                        .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                        .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                    },
                    .subresourceRange = {
                        .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                    }
                };

                REQUIRE_ZERO(vkCreateImageView(r->l, &viewInfo, NULL, &p->views[i]));
            } break;
            case SE_SwapImg:
            {
                SE_Log("Swap Attachment, Skipping\n");
            } break;
        }
    }

    



    //One framebuffer per pass, except final pass
    p->numframebuffers = r->s.numImgs + p->numSubpasses;//idk not necessary
    p->framebuffers = SE_HeapAlloc(sizeof(VkFramebuffer) * (p->numframebuffers));

    SE_render_pipeline_info* info = &p->pipelineInfo;
    
    u8* attachment_buf = SE_HeapAlloc(sizeof(u8) * p->num_attach);
    VkImageView* view_buf = SE_HeapAlloc(sizeof(VkImageView) * p->num_attach);
    SE_memset(attachment_buf, 0, sizeof(u8) * p->num_attach);
    u32 view_size = 0;
    u32 rp = 0;

    for (u32 i = 0; i < p->numSubpasses; i++) {

        if (info->passes[i].num == 0) {
            //early exit for final framebuffer special handling
            if (rp == p->numPasses - 1) break;

            //build framebuffer for pass
            SE_Log("RenderPass: %d, Attachments\n", rp); 
            for (u32 j = 0; j < p->num_attach; j++) {
                if (attachment_buf[j]) {
                    SE_Log("\tAttachmend idx: %d\n", j);
                    attachment_buf[j] = 0;
                    view_buf[view_size++] = p->views[j];
                }
            }

            VkFramebufferCreateInfo frameInfo = {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .pAttachments = view_buf,
                .attachmentCount = view_size,
                .renderPass = p->rpasses[rp].rp,
                .height = r->s.size.height,
                .width = r->s.size.width,
                .layers = 1,
            };

            REQUIRE_ZERO(vkCreateFramebuffer(r->l, &frameInfo, NULL, &p->framebuffers[i]));

            view_size = 0;
            rp++;
        }
        
        //attachment_buf[at_size++] = p->views[info->attach_refs[info->passes[i].start].attach_idx];
        for (u32 j = 0; j < info->passes[i].num; j++) {
            u32 index = info->attach_refs[info->passes[i].start + j].attach_idx;
            attachment_buf[index] = 1;
        }
    }

    //One Framebuffer per swapchain image
    //Maybe use a special second array for the swapchain framebuffers?
    //It might make the rest of the code a little nicer
    SE_Log("numimgs: %d\n", r->s.numImgs);
    for (u32 i = 0; i < r->s.numImgs; i++) {

        view_buf[view_size++] = r->s.views[i];
        for (u32 j = 1; j < p->num_attach; j++) {
            if (attachment_buf[j]) {
                view_buf[view_size++] = p->views[j];
            }
        }

        VkFramebufferCreateInfo frameInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pAttachments = view_buf,
            .attachmentCount = view_size,
            .renderPass = p->rpasses[rp].rp,
            .height = r->s.size.height,
            .width = r->s.size.width,
            .layers = 1,
        };
        view_size = 0;

        REQUIRE_ZERO(vkCreateFramebuffer(r->l, &frameInfo, NULL, &p->framebuffers[i]));
    }

    SE_HeapFree(view_buf);
    SE_HeapFree(attachment_buf);


}

//--------------------------------------------------------------------------------



//temporary
SE_DrawFrameFunc(SE_DrawFrame) {

    static u32 frame = 0;
    frame = (frame + 1) % r->s.numImgs;

    vkWaitForFences(r->l, 1, &p->s.pending[0], VK_TRUE, UINT64_MAX);
    vkResetFences(r->l, 1, &p->s.pending[0]);

    u32 img;
    REQUIRE_ZERO(vkAcquireNextImageKHR(r->l, r->s.swap, UINT64_MAX, p->s.avalible[frame], VK_NULL_HANDLE, &img));

    vkResetCommandBuffer(r->cmd, 0);
    VkCommandBufferBeginInfo cmdinfo = (VkCommandBufferBeginInfo) {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    REQUIRE_ZERO(vkBeginCommandBuffer(r->cmd, &cmdinfo));

    VkClearValue clearvalues[] = {
        {{0.2f, 0.2f, 0.2f, 1.0f}},
        {{1.0f, 1.0f, 1.0f, 1.0f}},
    };

    VkRenderPassBeginInfo rpassInfo = (VkRenderPassBeginInfo) {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderArea = (VkRect2D){
                .extent = r->s.size,
                .offset = (VkOffset2D){0, 0},
            },
            .renderPass = p->rpasses[0].rp,
            .framebuffer = p->framebuffers[img],
            .pClearValues = clearvalues,
            .clearValueCount = 1,
    };

    //SE_Log("SwapSize: (%d, %d)\n", r->s.size.width, r->s.size.height);


    vkCmdBeginRenderPass(r->cmd, &rpassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport view = (VkViewport) {
        .height = SE_DEFAULT_VIEW.height * r->s.size.height,
        .width = SE_DEFAULT_VIEW.width * r->s.size.width,
        .maxDepth = SE_DEFAULT_VIEW.depthRange + SE_DEFAULT_VIEW.depthBase,
        .minDepth = SE_DEFAULT_VIEW.depthBase
    };

    VkRect2D scissor = (VkRect2D) {
        .extent = {
            .width = r->s.size.width * SE_DEFAULT_SCISSOR.size.c.x,
            .height = r->s.size.height * SE_DEFAULT_SCISSOR.size.c.y
        },
        .offset = {
            .x = SE_DEFAULT_SCISSOR.offset.c.x * r->s.size.width, 
            .y = SE_DEFAULT_SCISSOR.offset.c.y * r->s.size.height
        },
    };

    vkCmdSetViewport(r->cmd, 0, 1, &view);
    vkCmdSetScissor(r->cmd, 0, 1, &scissor);

    //for (u32 i = 0; i < p->numSubpasses; i++) {
    {
        vkCmdBindPipeline(r->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, p->pipelines[0]);
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(r->cmd, 0, 1, (VkBuffer*)&vert->mem->resource, offsets);
        vkCmdDraw(r->cmd, 3, 1, 0, 0);
    }
    //}
        vkCmdNextSubpass(r->cmd, VK_SUBPASS_CONTENTS_INLINE);

    {
        vkCmdBindPipeline(r->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, p->pipelines[0]);
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(r->cmd, 0, 1, (VkBuffer*)&vert->mem->resource, offsets);
        vkCmdDraw(r->cmd, 3, 1, 0, 0);
    }


    vkCmdEndRenderPass(r->cmd);
    REQUIRE_ZERO(vkEndCommandBuffer(r->cmd));

    VkPipelineStageFlags flags[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo submitInfo = (VkSubmitInfo) {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pWaitSemaphores = &p->s.avalible[frame],
            .waitSemaphoreCount = 1,

            .pWaitDstStageMask = flags,

            .pCommandBuffers = &r->cmd,
            .commandBufferCount = 1,

            .pSignalSemaphores = &p->s.finished[img],
            .signalSemaphoreCount = 1,

    };

    REQUIRE_ZERO(vkQueueSubmit(r->Queues.g, 1, &submitInfo, p->s.pending[0]));

    VkPresentInfoKHR presentInfo = (VkPresentInfoKHR){
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pSwapchains = &r->s.swap,
            .swapchainCount = 1,

            .pWaitSemaphores = &p->s.finished[img],
            .waitSemaphoreCount = 1,

            .pImageIndices = &img,

            .pResults = NULL,
    };

    REQUIRE_ZERO(vkQueuePresentKHR(r->Queues.p, &presentInfo));

}
