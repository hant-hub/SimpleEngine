#ifndef FUNCDEF_H
#define FUNCDEF_H

#include <platform.h>

#include <render/render.h>
#include <render/vertex.h>
#include <render/pipeline.h>
#include <render/memory.h>
#include <render/utils.h>


/* -----------------------------+
 *        Render Functions      |
 *                              |
 * -----------------------------+ */

//Core Funcs ----------------------------------
#define SE_CreateRenderContextFunc(x) \
    SE_render_context (x)(SE_window* win)

#define SE_CreateSwapChainFunc(x) \
    SE_swapchain (x)(SE_allocator* a, SE_render_context* r, SE_window* win, SE_swapchain* old)

//GPU Memory Functions ------------------------
#define SE_CreateHeapTrackersFunc(x) \
    SE_render_memory (x)(SE_render_context* r)

#define SE_TransferMemoryFunc(x) \
    void (x)(SE_render_context* r, SE_render_buffer dst, const void* data, u64 size)

#define SE_CreateResourceTrackerBufferFunc(x) \
    SE_resource_arena (x)(SE_render_context* r, VkBufferUsageFlagBits flags, VkMemoryPropertyFlagBits props, u64 minsize)

#define SE_CreateBufferFunc(x) \
    SE_render_buffer (x)(SE_resource_arena* a, u64 size)

#define SE_CreateResourceTrackerRawFunc(x) \
    SE_resource_arena (x)(SE_render_context* r, VkMemoryPropertyFlagBits props, u64 minsize)

#define SE_CreateRawFunc(x) \
    u64 (x)(SE_resource_arena* a, u64 size)

//Cache Funcs --------------------------
#define SE_InitPipelineCacheFunc(x) \
    SE_pipeline_cache (x)(const SE_allocator a)

#define SE_PushPipelineTypeFunc(x) \
    void (x)(SE_pipeline_cache* c, SE_pipeline_options o)

#define SE_FreePipelineCacheFunc(x) \
    void (x)(SE_pipeline_cache* c)

//Pipeline Funcs -------------------
#define SE_LoadShaderFunc(x) \
    SE_shaders (x) (SE_render_context* r, const char* vert, const char* frag)

#define SE_CreateSyncObjsFunc(x) \
    SE_sync_objs (x)(const SE_render_context* r)


#define SE_BeginPipelineCreationFunc(x) \
    SE_render_pipeline (x)(void)

#define SE_EndPipelineCreationFunc(x) \
    void (x)(SE_render_context* r, SE_render_pipeline* p, const SE_pipeline_cache* c)


#define SE_AddShaderFunc(x) \
    u32 (x)(SE_render_pipeline* pipe, SE_shaders* s, u32 pipeline)

#define SE_AddVertSpecFunc(x) \
    u32 (x)(SE_render_pipeline* pipe, SE_vertex_spec* v)

#define SE_AddDepthAttachmentFunc(x) \
    u32 (x)(SE_render_pipeline* pipe)

#define SE_OpqaueNoDepthPassFunc(x) \
    void (x)(SE_render_pipeline* pipe, u32 vert, u32 target, u32 shader)

#define SE_OpaquePassFunc(x) \
    void (x)(SE_render_pipeline* pipe, u32 vert, u32 target, u32 depth, u32 shader)

#define SE_CreateFrameBuffersFunc(x) \
    void (x)(const SE_render_context* r, SE_render_pipeline* p)

#define SE_DrawFrameFunc(x) \
    void (x)(SE_window* win, SE_render_context* r, SE_render_pipeline* p, SE_render_buffer* vert)

//Vertex Functions -------------------------------------------
#define SE_CreateVertSpecInlineFunc(x) \
    SE_vertex_spec (x)(SE_allocator* a, SE_struct_member* mem, u64 size)



typedef SE_LoadShaderFunc(*SE_load_shader_func);
typedef SE_CreateVertSpecInlineFunc(*SE_create_vert_spec_inline_func);
typedef SE_CreateResourceTrackerBufferFunc(*SE_create_resource_tracker_buffer_func);
typedef SE_CreateBufferFunc(*SE_create_buffer_func);
typedef SE_TransferMemoryFunc(*SE_transfer_memory_func);
typedef SE_InitPipelineCacheFunc(*SE_init_pipeline_cache_func);
typedef SE_PushPipelineTypeFunc(*SE_push_pipeline_type_func);
typedef SE_FreePipelineCacheFunc(*SE_free_pipeline_cache_func);
typedef SE_AddShaderFunc(*SE_add_shader_func);
typedef SE_AddVertSpecFunc(*SE_add_vert_spec_func);
typedef SE_AddDepthAttachmentFunc(*SE_add_depth_attachment_func);
typedef SE_BeginPipelineCreationFunc(*SE_begin_pipeline_creation_func);
typedef SE_OpqaueNoDepthPassFunc(*SE_opaque_no_depth_pass_func);
typedef SE_OpaquePassFunc(*SE_opaque_pass_func);
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


#endif
