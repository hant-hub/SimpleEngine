#ifndef SE_PIPELINE_CACHE_H
#define SE_PIPELINE_CACHE_H

#include "vulkan/vulkan_core.h"
#include <util.h>
#include <math/vector.h>
#include <render/vertex.h>
#include <vulkan/vulkan.h>

typedef struct SE_dynamic_state_info {
    //Bool32 View; Stored in Viewport state for now
    //INFO: By storing in Viewport it can be made to be
    //0 initialized to dynamic viewport, rather than
    //static.
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
    SE_cull_mode cull_mode;
    SE_front winding;

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

//mappings ---------------------------------
static const VkPrimitiveTopology SE_primitive_map[] = {
    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
    VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
    //More Later
};

static const VkFrontFace SE_frontface_map[] = {
    VK_FRONT_FACE_CLOCKWISE,
    VK_FRONT_FACE_COUNTER_CLOCKWISE,
};

static const VkPolygonMode SE_polygon_mode_map[] = {
    VK_POLYGON_MODE_FILL,
    VK_POLYGON_MODE_LINE,
};

static const VkCullModeFlags SE_cull_mode_map[] = {
    VK_CULL_MODE_BACK_BIT,
    VK_CULL_MODE_FRONT_BIT,
    VK_CULL_MODE_NONE,
};

static const VkCompareOp SE_compare_map[] = {
    VK_COMPARE_OP_LESS,
    VK_COMPARE_OP_GREATER,
    VK_COMPARE_OP_EQUAL,
    VK_COMPARE_OP_ALWAYS,
};

//NOTE(ELI):
//Both view and scissor are in terms of fractions of the
//screen. To calculate the actual viewport and scissor you
//have to multiply the values by the swapchain img size.
//
//That just how it is so idk man. It doesn't make sense to
//bake in a viewport that doesn't scale with the screen,
//However since its a float I will probably add that option
//at some point.
typedef struct SE_view {
    float width;
    float height; 
    float depthRange;
    float depthBase;
} SE_view;

typedef struct SE_scissor {
    SE_v2f size;
    SE_v2f offset;
} SE_scissor;

typedef struct SE_viewport_state {
    VkBool32 enableStatic;
    SE_view view;
    SE_scissor scissor;
} SE_viewport_state;


const static SE_view SE_DEFAULT_VIEW = {
    .width = 1.0f,
    .height = 1.0f,
    .depthBase = 0.0f,
    .depthRange = 1.0f,
};

const static SE_scissor SE_DEFAULT_SCISSOR = {
    .size = {1.0f, 1.0f},
    .offset = {0.0f, 0.0f}
};

typedef struct SE_pipeline_options {
    //VkPipelineShaderStageCreateInfo           pStages;
    u32 shaders;
    SE_viewport_state view;
    SE_dynamic_state_info dyn;
    SE_vertex_spec spec;
    SE_primitive_type t; //NOTE(ELI): Implicit primitive restart

    //TODO(ELI): Tesselation Support
    //VkPipelineTessellationStateCreateInfo     pTessellationState;

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

#endif
