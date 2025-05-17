#ifndef SE_PIPELINE_CACHE_H
#define SE_PIPELINE_CACHE_H

#include <util.h>
#include <math/vector.h>
#include <render/vertex.h>
#include <vulkan/vulkan.h>

typedef struct SE_dynamic_state_info {
    Bool32 View;
    Bool32 Scissor;
    Bool32 LineWidth;
    Bool32 DepthBias;
    Bool32 BlendConstants;
    // More stuff as needed
} SE_dynamic_state_info;

typedef enum SE_primitive_type {
    SE_TRIANGLE_LIST = 0, //default
    SE_POINT_LIST,
    SE_LINE_LIST,
    //More later
} SE_primitive_type;

typedef struct SE_draw_region {
    SE_v2f offset;
    SE_v2f size;
} SE_draw_region;

typedef struct SE_view {
    SE_v2f offset;
    SE_v2f size;

    SE_v2f depth;
} SE_view;

static const SE_view SE_DEFAULT_VIEW = (SE_view) {
    .offset = {0, 0},
    .size = {1, 1}, //fullscreen
    .depth = {0, 1},
};

static const SE_draw_region SE_DEFAULT_SCISSOR = (SE_draw_region) {
    .offset = {0, 0},
    .size = {1, 1}, //full screen
};

typedef enum SE_polygon_mode {
    SE_POLYGON_FILL = 0,
    SE_POLYGON_LINE,
} SE_polygon_mode;

typedef enum SE_cull_mode {
    SE_CULL_BACK = 0,
    SE_CULL_FRONT,
    SE_NONE,
} SE_cull_mode;

typedef enum SE_front {
    SE_WIND_CLOCKWISE = 0,
    SE_WIND_COUNTERCLOCK,

} SE_front;

typedef struct SE_raster_info {
    Bool32 depthClamp;
    Bool32 rasterDiscard;
    Bool32 depthBias;
    SE_polygon_mode polymode;

    //default zero
    float depthBiasConstantFactor;
    float depthBiasClamp;
    float depthBiasSlopeFactor;
    float lineWidth;
} SE_raster_info;

typedef enum SE_depth_op {
    SE_COMPARE_LESS = 0,
    SE_COMPARE_GREATER,
    SE_COMPARE_EQUAL,
    SE_COMPARE_ALWAYS,
} SE_depth_op;

typedef struct SE_depth_info {
    Bool32 depthTestEnable;
    Bool32 depthWriteEnable;
    SE_depth_op op;
} SE_depth_info;

typedef struct SE_pipeline_options {
    //VkPipelineShaderStageCreateInfo           pStages;
    u32 shaders;
    SE_dynamic_state_info dyn;
    SE_vertex_spec spec;
    SE_primitive_type t; //NOTE(ELI): Implicit primitive restart

    //TODO(ELI): Tesselation Support
    //VkPipelineTessellationStateCreateInfo     pTessellationState;

    SE_draw_region scissor;
    SE_view view;
    SE_raster_info raster;

    //TODO(ELI): Multisample support
    //VkPipelineMultisampleStateCreateInfo      pMultisampleState;

    SE_depth_info depth; //currently no stencil info

    //TODO(ELI): Transparency Support
    //VkPipelineColorBlendStateCreateInfo       pColorBlendState;
} SE_pipeline_options;


//Table of Pipelines, will then be
//matched with shaders.
typedef struct SE_pipeline_cache {
    SE_pipeline_options* pipelines; //construct pipelines at end
    SE_allocator mem;
    u32 cap;
    u32 size;
} SE_pipeline_cache;

//void SE_PushPipelineType(void);

#endif
