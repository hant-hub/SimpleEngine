#include "render/cache.h"
#include "render/memory.h"
#include "util.h"
#include "vulkan/vulkan_core.h"
#include <platform.h>
#include <render/render.h>
#include <render/pipeline.h>

#include <math/mat.h>

#include "init.h"
#include <stdio.h>
#include <user.h>
#include <generated/generated_struct.h>

static SE_shaders sh;
static SE_resource_arena vertex;
static SE_render_buffer vbuf;
static SE_sync_objs sync;
static SE_render_pipeline pipe;

const static vert verts[] = {
    (vert){{0.0, -0.5, 0.5f},   {0, 0}},
    (vert){{0.5, 0.5, 0.5f},    {0, 0}},
    (vert){{-0.5, 0.5, 0.5f},   {0, 0}},
};



SE_INIT_FUNC(Init) {
   sh = s->LoadShaders(&s->r, "shaders/vert.spv", "shaders/frag.spv");

   SE_allocator a = (SE_allocator){
        .alloc = s->StaticArenaAlloc,
        .ctx = s->HeapArenaCreate(KB(2))
   };

   SE_vertex_spec sv = s->CreateVertSpecInline(&a, Meta_Def_vert, ASIZE(Meta_Def_vert));

   vertex = s->CreateResourceTrackerBuffer(&s->r,
           VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, MB(3));

   vbuf = s->CreateBuffer(&vertex, sizeof(verts));
   s->TransferMemory(&s->r, vbuf, verts, sizeof(verts));

    /* Mockup render pipeline creation
    PipelineCache cache = {0};
    CreatePipeline(&cache, options ...);
    CreatePipeline(&cache, options ...);
    CreatePipeline(&cache, options ...);

    SE_shader s = Createshader(file, cache);

    PipelineInfo pinfo = BeginPipelineCreation();
    BeginPass(color);

    int depth = MakeDepthAttachment();
    int color = MakeColorAttachment();

    AddOpaquePass(pinfo, depth, shaders); //additive
    AddTransparentPass(pinfo, depth, shaders); //additive
    AddText(pinfo);

    EndPass();

    int stage1 = MakeColorAttachment();
    AddPostEffect(pinfo, stage1, color, shaders); //implicit Renderpasses
    AddPostEffect(pinfo, SE_SCREEN, stage1, shaders);

    Pipeline p = EndPipelineCreation(pinfo, cache);


    ...
    renderpipeline(p);
     
    */

   SE_allocator global = {
       .alloc = s->HeapGlobalAlloc,
   };


   SE_render_pipeline_info p = s->BeginPipelineCreation(); 

   u32 vidx = s->AddVertSpec(&p, &sv);
   u32 sidx = s->AddShader(&p, &sh, 0);

   u32 depth = s->AddDepthAttachment(&p);

   SE_pipeline_cache c = s->InitPipelineCache(global);

   s->PushPipelineType(&c, (SE_pipeline_options){
           .shaders = sidx, 
           .view = {
                .view = SE_DEFAULT_VIEW,
                .scissor = SE_DEFAULT_SCISSOR
           },
   });

   s->PushPipelineType(&c, (SE_pipeline_options){
           .shaders = sidx, 
           .depth = {
                .depthTestEnable = TRUE,
                .depthWriteEnable = TRUE,
           },
           .view = {
                .view = SE_DEFAULT_VIEW,
                .scissor = SE_DEFAULT_SCISSOR
           },
   });


   //s->OpaquePass(&p, 0, 0, 1, sidx);
   s->OpaqueNoDepthPass(&p, 0, 0, sidx);

   printf("RenderPass Info:\n");
   for (u32 i = 0; i < p.psize; i++) {
       printf("\t(%d, %d)\n", p.passes[i].start, p.passes[i].num);
   }

   pipe = s->EndPipelineCreation(&s->r, &p, &c);
   s->FreePipelineCache(&c);
   s->HeapFree(a.ctx);
}

SE_UPDATE_FUNC(Update) {
}

SE_DRAW_FUNC(Draw) {
    s->DrawFrame(s->w, &s->r, &pipe, &vbuf);
}


SE_DLLEXPORT SE_user_state state = {
    .init = Init,
    .update = Update,
    .draw = Draw,
};
