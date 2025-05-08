#include "render/memory.h"
#include <platform.h>
#include <render/render.h>
#include <render/pipeline.h>

#include <math/mat.h>

#include "init.h"
#include <stdio.h>
#include <user.h>
#include <generated/generated_struct.h>

static SE_shaders sh;
static SE_render_pipeline p;
static SE_resource_arena vertex;
static SE_render_buffer vbuf;
static SE_sync_objs sync;

SE_INIT_FUNC(Init) {
   sh = s->LoadShaders(&s->r, "shaders/vert.spv", "shaders/frag.spv");

   SE_mem_arena config = s->ArenaCreateHeap(KB(10));

   SE_vertex_spec sv = s->CreateVertSpecInline(&config, Meta_Def_vert, ASIZE(Meta_Def_vert));
   p = s->CreatePipeline(s->ArenaCreateHeap(MB(10)), &s->r, &sv, &sh);

   s->ArenaDestroyHeap(config);

   vertex = s->CreateResourceTrackerBuffer(&s->r, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                                 KB(2));
   vert vdata[] = { 
       (vert) { .pos = {-0.6, 0.5, 0.2}, .uv = {0, 1}, },
       (vert) { .pos = {0.3, -0.5, 0.2}, .uv = {0, 1}, },
       (vert) { .pos = {0.8, 0.5,  0.2}, .uv = {0, 1}, },
       (vert) { .pos = {-0.5, 0.5, 0.4}, .uv = {1, 0}, },
       (vert) { .pos = {0,   -0.5, 0.4}, .uv = {1, 0}, },
       (vert) { .pos = {0.5,  0.5, 0.4}, .uv = {1, 0}, },
   };

   vbuf = s->CreateBuffer(&vertex, sizeof(vdata));
   s->TransferMemory(&s->r, vbuf, vdata, sizeof(vdata));

   sync = s->CreateSyncObjs(&s->r);
}

SE_UPDATE_FUNC(Update) {
}

SE_DRAW_FUNC(Draw) {
    s->DrawFrame(s->w, &s->r, &p, &sync, &vertex);
}


SE_DLLEXPORT SE_user_state state = {
    .init = Init,
    .update = Update,
    .draw = Draw,
};
