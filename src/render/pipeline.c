#include "render/utils.h"
#include "render/vertex.h"
#include "util.h"
#include <assert.h>
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

    VkPipelineLayoutCreateInfo layoutinfo = (VkPipelineLayoutCreateInfo){
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pushConstantRangeCount = 0,
    };
    REQUIRE_ZERO(vkCreatePipelineLayout(r->l, &layoutinfo, NULL, &s.layout));

    SE_Log("Shaders Loaded: %s %s\n", vert, frag);
    return s;
}

void SE_CreateFrameBuffers(SE_render_context* r, SE_render_pipeline* p) {
    p->numframebuffers = r->s.numImgs;
    p->framebuffers = SE_HeapAlloc(sizeof(VkFramebuffer) * r->s.numImgs);
    for (u32 i = 0; i < r->s.numImgs; i++) {
        VkImageView views[] = {
            r->s.views[i],
            p->views[1]
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
    VkRenderPassCreateInfo i;

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

//-------------------------------------------------------------

/*
 *  The final pass will use index zero always
 *
 */
SE_render_pipeline_info SE_BeginPipelineCreation(void) {
    SE_render_pipeline_info info = (SE_render_pipeline_info){0};
    SE_push_attachment(&info, SE_SwapImg);

    return info;
}

void SE_OpqaueNoDepthPass(SE_render_pipeline_info* p, u32 target, SE_shaders shader) {
    SE_subpass_attach a = (SE_subpass_attach) {
        .usage = SE_attach_color,
        .attach_idx = target,
    };

    SE_sub_pass s = (SE_sub_pass) {
        
        .start = p->rsize,
        .num = 1,
    };

    SE_push_attach_ref(p, a); 
    SE_push_pass(p, s);
}

SE_render_pipeline SE_EndPipelineCreation(const SE_render_context* r, const SE_render_pipeline_info* info) {
    SE_render_pipeline p = {0};
    p.mem.ctx = SE_HeapArenaCreate(KB(10));
    p.mem.alloc = SE_StaticArenaAlloc;

    SE_mem_arena* m = SE_HeapArenaCreate(KB(12));
    SE_allocator a = (SE_allocator) {
        .alloc = SE_StaticArenaAlloc,
        .ctx = m
    };

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


    //Each step needs to build the description and the
    //reference, then finalize the subpass
    for (u32 i = 0; i < info->psize; i++) {
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
                        descrip[curr_descrip] = (VkAttachmentDescription) {
                            .format = r->s.format.format,
                            .samples = VK_SAMPLE_COUNT_1_BIT,
                            .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                            .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                        };

                        colors[curr_colors++] = (VkAttachmentReference) {
                            .attachment = curr_descrip,
                            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                        };

                        subDep[i].srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                        subDep[i].dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                        subDep[i].dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    } break;
                case SE_attach_depth:
                    {
                        SE_Log("Not Implemented\n");
                        assert(0);
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
            curr_descrip++;
        }
        
        subDes[i] = (VkSubpassDescription) {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,

            .colorAttachmentCount = curr_colors - start_colors,
            .pColorAttachments = &colors[start_colors],

            .inputAttachmentCount = curr_input - start_input,
            .pInputAttachments = &input[start_input],

            .pResolveAttachments = NULL, //would be the same as color attachments but with multisample buffers
                                         
            //TODO(ELI): Rework later to take advantage of single depth attachment
            .pDepthStencilAttachment = (curr_depth - start_depth) ? &depth[curr_depth]: NULL,
        };
    }

    VkRenderPassCreateInfo rinfo = (VkRenderPassCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .subpassCount = info->psize,
        .dependencyCount = info->psize,
        .pSubpasses = subDes,
        .pDependencies = subDep,
        .attachmentCount = info->atsize,
        .pAttachments = descrip,
    };


    VkGraphicsPipelineCreateInfo _;

    SE_Log("atsize: %d\n", info->atsize);

    p.numPasses = 1;
    p.rpasses = p.mem.alloc(0, sizeof(SE_render_pass) * p.numPasses, NULL, p.mem.ctx);
    REQUIRE_ZERO(vkCreateRenderPass(r->l, &rinfo, NULL, &p.rpasses[0].rp));



    SE_HeapFree(m);
    return p;
}

//temporary
void SE_DrawFrame(SE_window* win, SE_render_context* r, SE_render_pipeline* p, SE_sync_objs* s, SE_resource_arena* vert) {
}
