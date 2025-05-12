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
    SE_vertex_spec (x)(SE_mem_arena* a, SE_struct_member* mem, u64 size)

#define SE_CreatePipelineFunc(x) \
    SE_render_pipeline (x)(SE_mem_arena a, SE_render_context* r, SE_vertex_spec* vspec, SE_shaders* s)

#define SE_CreateResourceTrackerBufferFunc(x) \
    SE_resource_arena (x)(SE_render_context* r, VkBufferUsageFlagBits flags, VkMemoryPropertyFlagBits props, u64 minsize)

#define SE_CreateBufferFunc(x) \
    SE_render_buffer (x)(SE_resource_arena* a, u64 size)

#define SE_TransferMemoryFunc(x) \
    void (x)(SE_render_context* r, SE_render_buffer dst, void* data, u64 size)

#define SE_BeginPipelineCreationFunc(x) \
    SE_render_pipeline_info (x)(void)

#define SE_OpqaueNoDepthPassFunc(x) \
    void (x)(SE_render_pipeline_info* p, u32 target, SE_shaders shader)

#define SE_EndPipelineCreationFunc(x) \
    SE_render_pipeline (x)(const SE_render_context* r, const SE_render_pipeline_info* info)

#define SE_CreateSyncObjsFunc(x) \
    SE_sync_objs (x)(SE_render_context* r)

#define SE_DrawFrameFunc(x) \
    void (x)(SE_window* win, SE_render_context* r, SE_render_pipeline* p, SE_sync_objs* s, SE_resource_arena* vert)

typedef SE_LoadShaderFunc(*SE_load_shader_func);
typedef SE_CreateVertSpecInlineFunc(*SE_create_vert_spec_inline_func);
typedef SE_CreatePipelineFunc(*SE_create_pipeline);
typedef SE_CreateResourceTrackerBufferFunc(*SE_create_resource_tracker_buffer_func);
typedef SE_CreateBufferFunc(*SE_create_buffer_func);
typedef SE_TransferMemoryFunc(*SE_transfer_memory_func);
typedef SE_BeginPipelineCreationFunc(*SE_begin_pipeline_creation_func);
typedef SE_OpqaueNoDepthPassFunc(*SE_opque_no_depth_pass_func);
typedef SE_EndPipelineCreationFunc(*SE_end_pipeline_creation_func);
typedef SE_CreateSyncObjsFunc(*SE_create_sync_objs_func);
typedef SE_DrawFrameFunc(*SE_draw_frame_func);

//Memory Functions ---------------------------------------
#define SE_ArenaCreateHeapFunc(x) \
    SE_mem_arena (x)(u32 size)

#define SE_ArenaDestroyHeapFunc(x) \
    void (x)(SE_mem_arena a)

typedef SE_ArenaCreateHeapFunc(*SE_arena_create_heap_func);
typedef SE_ArenaDestroyHeapFunc(*SE_arena_destroy_heap_func);


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

    SE_begin_pipeline_creation_func BeginPipelineCreation;
    SE_opque_no_depth_pass_func OpqaueNoDepthPass;
    SE_end_pipeline_creation_func EndPipelineCreation;

    SE_create_sync_objs_func CreateSyncObjs;
    SE_draw_frame_func DrawFrame;

    //Memory Functions
    SE_arena_create_heap_func ArenaCreateHeap;
    SE_arena_destroy_heap_func ArenaDestroyHeap;
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
