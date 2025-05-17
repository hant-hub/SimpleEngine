#include "render/memory.h"
#include "util.h"
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

SE_INIT_FUNC(Init) {
   sh = s->LoadShaders(&s->r, "shaders/vert.spv", "shaders/frag.spv");

   SE_allocator a = (SE_allocator){
        .alloc = s->StaticArenaAlloc,
        .ctx = s->HeapArenaCreate(KB(2))
   };

   SE_vertex_spec sv = s->CreateVertSpecInline(&a, Meta_Def_vert, ASIZE(Meta_Def_vert));

   s->HeapFree(a.ctx);

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

    SE_render_pipeline_info p = s->BeginPipelineCreation(); 
    s->OpqaueNoDepthPass(&p, 0, sh);

    for (u32 i = 0; i < p.psize; i++) {
        printf("(%d, %d) ", p.passes[i].start, p.passes[i].num);
    }
    printf("\n");

    SE_render_pipeline pipe = s->EndPipelineCreation(&s->r, &p);
}

SE_UPDATE_FUNC(Update) {
}

SE_DRAW_FUNC(Draw) {
    //s->DrawFrame(s->w, &s->r, &p, &sync, &vertex);
}


SE_DLLEXPORT SE_user_state state = {
    .init = Init,
    .update = Update,
    .draw = Draw,
};
