#include "util.h"
#include <render/cache.h>


SE_pipeline_cache SE_InitPipelineCache(const SE_allocator a) {
    SE_pipeline_cache c = (SE_pipeline_cache){
        .mem = a,
    };
    return c;
}

void SE_PushPipelineType(SE_pipeline_cache* c, SE_pipeline_options o) {
    if (c->size + 1 > c->cap) {
        c->cap = c->cap ? c->cap * 2 : 1;
        c->pipelines = c->mem.alloc(
                sizeof(SE_pipeline_options) * c->size,
                sizeof(SE_pipeline_options) * c->cap,
                c->pipelines,
                c->mem.ctx);
    }
    c->pipelines[c->size++] = o;
}

void SE_FreePipelineCache(SE_pipeline_cache* c) {
    c->mem.alloc(sizeof(SE_pipeline_options) * c->cap,
                 0,
                 c->pipelines,
                 c->mem.ctx);
}
