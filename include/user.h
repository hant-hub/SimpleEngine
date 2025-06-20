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

#include "funcdefs.h"

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
    SE_add_depth_attachment_func AddDepthAttachment;

    SE_free_pipeline_cache_func FreePipelineCache;
    SE_begin_pipeline_creation_func BeginPipelineCreation;
    SE_opaque_no_depth_pass_func OpaqueNoDepthPass;
    SE_opaque_pass_func OpaquePass;
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
