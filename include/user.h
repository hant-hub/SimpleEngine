#ifndef SE_USER_H
#define SE_USER_H
/*
 *
 * Header File for Communication between the engine and the 
 * Application DLL. 
 *
 * Here should be shared data structure definitions, as well
 * as function pointer definitions etc.
 *
 */

#include "platform/wayland/wayland.h"
#include "render/cache.h"
#include "util.h"
#include <platform.h>

#include <render/render.h>
#include <render/vertex.h>
#include <render/pipeline.h>
#include <render/memory.h>
#include <render/utils.h>

struct SE;

//User Space -------------------------------------------

#define SE_INIT_FUNC(x) void(x)(struct SE* s)
#define SE_UPDATE_FUNC(x) void(x)(struct SE* s)
#define SE_DRAW_FUNC(x) void(x)(struct SE* s)

typedef SE_INIT_FUNC(*SE_init_func);
typedef SE_UPDATE_FUNC(*SE_update_func);
typedef SE_DRAW_FUNC(*SE_draw_func);

typedef struct SE_user_state {
    SE_init_func init;
    SE_update_func update;
    SE_draw_func draw;
} SE_user_state;


//Render Functions --------------------------------------
#define SE_LoadShaderFunc(x) \
    SE_shaders (x) (SE_render_context* r, const char* vert, const char* frag)

#define SE_CreateVertSpecInlineFunc(x) \
    SE_vertex_spec (x)(SE_allocator* a, SE_struct_member* mem, u64 size)

#define SE_CreatePipelineFunc(x) \
    SE_render_pipeline (x)(SE_allocator a, SE_render_context* r, SE_vertex_spec* vspec, SE_shaders* s)

#define SE_CreateResourceTrackerBufferFunc(x) \
    SE_resource_arena (x)(SE_render_context* r, VkBufferUsageFlagBits flags, VkMemoryPropertyFlagBits props, u64 minsize)

#define SE_CreateBufferFunc(x) \
    SE_render_buffer (x)(SE_resource_arena* a, u64 size)

#define SE_TransferMemoryFunc(x) \
    void (x)(SE_render_context* r, SE_render_buffer dst, const void* data, u64 size)

#define SE_InitPipelineCacheFunc(x) \
    SE_pipeline_cache (x)(const SE_allocator a)

#define SE_FreePipelineCacheFunc(x) \
    void (x)(SE_pipeline_cache* c)

#define SE_PushPipelineTypeFunc(x) \
    void (x)(SE_pipeline_cache* c, SE_pipeline_options o)

#define SE_AddShaderFunc(x) \
    u32 (x)(SE_render_pipeline_info* p, SE_shaders* s)

#define SE_AddVertSpecFunc(x) \
    u32 (x)(SE_render_pipeline_info* p, SE_vertex_spec* v)

#define SE_BeginPipelineCreationFunc(x) \
    SE_render_pipeline_info (x)(void)

#define SE_OpqaueNoDepthPassFunc(x) \
    void (x)(SE_render_pipeline_info* p, u32 vert, u32 target, u32 shader)

#define SE_EndPipelineCreationFunc(x) \
    SE_render_pipeline (x)(const SE_render_context* r, const SE_render_pipeline_info* info, const SE_pipeline_cache* c)

#define SE_CreateSyncObjsFunc(x) \
    SE_sync_objs (x)(const SE_render_context* r)

#define SE_DrawFrameFunc(x) \
    void (x)(SE_window* win, SE_render_context* r, SE_render_pipeline* p, SE_resource_arena* vert)

typedef SE_LoadShaderFunc(*SE_load_shader_func);
typedef SE_CreateVertSpecInlineFunc(*SE_create_vert_spec_inline_func);
typedef SE_CreatePipelineFunc(*SE_create_pipeline);
typedef SE_CreateResourceTrackerBufferFunc(*SE_create_resource_tracker_buffer_func);
typedef SE_CreateBufferFunc(*SE_create_buffer_func);
typedef SE_TransferMemoryFunc(*SE_transfer_memory_func);
typedef SE_InitPipelineCacheFunc(*SE_init_pipeline_cache_func);
typedef SE_PushPipelineTypeFunc(*SE_push_pipeline_type_func);
typedef SE_FreePipelineCacheFunc(*SE_free_pipeline_cache_func);
typedef SE_AddShaderFunc(*SE_add_shader_func);
typedef SE_AddVertSpecFunc(*SE_add_vert_spec_func);
typedef SE_BeginPipelineCreationFunc(*SE_begin_pipeline_creation_func);
typedef SE_OpqaueNoDepthPassFunc(*SE_opque_no_depth_pass_func);
typedef SE_EndPipelineCreationFunc(*SE_end_pipeline_creation_func);
typedef SE_CreateSyncObjsFunc(*SE_create_sync_objs_func);
typedef SE_DrawFrameFunc(*SE_draw_frame_func);

//Memory Functions ---------------------------------------

#define SE_HeapAllocFunc(x) \
    void* (x)(u64 size)

#define SE_HeapReallocFunc(x) \
    void* (x)(void* p, u64 size)

#define SE_HeapFreeFunc(x) \
    void (x)(void* p)

#define SE_HeapArenaCreateFunc(x) \
    SE_mem_arena* (x)(u64 size)

typedef SE_HeapAllocFunc(*SE_heap_alloc_func);
typedef SE_HeapReallocFunc(*SE_heap_realloc_func);
typedef SE_HeapFreeFunc(*SE_heap_free_func);
typedef SE_HeapArenaCreateFunc(*SE_heap_arena_create_func);

//API ----------------------------------------------------
typedef struct SE {
    //Engine State
    SE_window* w;
    SE_render_context r;
    SE_user_state user;

    //Render Functions
    SE_load_shader_func LoadShaders;
    SE_create_vert_spec_inline_func CreateVertSpecInline;
    SE_create_resource_tracker_buffer_func CreateResourceTrackerBuffer;
    SE_create_buffer_func CreateBuffer;
    SE_transfer_memory_func TransferMemory;

    SE_init_pipeline_cache_func InitPipelineCache;
    SE_push_pipeline_type_func PushPipelineType;

    SE_add_shader_func AddShader;
    SE_add_vert_spec_func AddVertSpec;

    SE_free_pipeline_cache_func FreePipelineCache;
    SE_begin_pipeline_creation_func BeginPipelineCreation;
    SE_opque_no_depth_pass_func OpqaueNoDepthPass;
    SE_end_pipeline_creation_func EndPipelineCreation;

    SE_create_sync_objs_func CreateSyncObjs;
    SE_draw_frame_func DrawFrame;

    //Memory Functions
    
    SE_heap_alloc_func HeapAlloc;
    SE_heap_realloc_func HeapRealloc;
    SE_heap_free_func HeapFree;

    SE_heap_arena_create_func HeapArenaCreate;
    SE_alloc_func StaticArenaAlloc;
    SE_alloc_func HeapGlobalAlloc;

} SE;



//DLL Macros ----------------------------------------------------------


#ifdef WIN32
    #define SE_DLLEXPORT __declspec(dllexport)
    #define SE_DLLHIDE 
#else
    #define SE_DLLEXPORT __attribute__ ((visibility("default")))
    #define SE_DLLHIDE __attribute__ ((visibility("hidden")))
#endif



#endif
