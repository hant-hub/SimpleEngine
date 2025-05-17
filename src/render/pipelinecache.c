#include "util.h"
#include "vulkan/vulkan_core.h"
#include <render/cache.h>


SE_pipeline_cache SE_InitPipeline(const SE_allocator a) {
    SE_pipeline_cache c = (SE_pipeline_cache){
        .mem = a
    };
    return c;
}

void SE_PushPipelineType(void) {

}
