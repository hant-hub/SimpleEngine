#ifndef SE_GRAPHICS_H
#define SE_GRAPHICS_H

#include "cutils.h"
#include <core/introspect.h>

//TODO(ELI): Replace flags with engine provided types
//TODO(ELI): Custom logic stuff
#include "vulkan/vulkan_core.h"
typedef struct SEGraphicsSettings {
    struct {
        bool8 set;
        u64 size;
    } minMemory;

    struct {
        bool8 set;
        VkImageUsageFlagBits* imgflags;
        u32 num_flags;
    } minMemoryImg; 

    struct {
        bool8 set;
        VkBufferUsageFlagBits* bufflags;
        u32 num_flags;
    } minMemoryBuf; 
} SEGraphicsSettings;

typedef struct SEwindow SEwindow;

//Rendergraph
typedef struct SERenderPipelineInfo SERenderPipelineInfo;
typedef struct SERenderPipeline SERenderPipeline;
typedef struct SECmdBuf SECmdBuf;

//rendercallback
typedef void (*SEDrawFunc)(SECmdBuf* p, void* pass);

#define SE_SCREEN 0

void DrawTriangle(SECmdBuf* buf, void* pass);

SERenderPipelineInfo* SECreatePipeline(SEwindow* win);
void SEBeginRenderPass(SERenderPipelineInfo* r);
void SEEndRenderPass(SERenderPipelineInfo* r);

u32 SEAddResource(SERenderPipelineInfo* r, bool8 clear);
u32 SEAddShader(SEwindow* win, SERenderPipelineInfo* r, SString source);
void SEBindShaders(SERenderPipelineInfo* r, u32 vert, u32 frag, u32 layout);
u32 SEAddLayout(SEwindow* win, SERenderPipelineInfo* r);

void SEReadResource(SERenderPipelineInfo* r, u32 resourceID);
void SEWriteResource(SERenderPipelineInfo* r, u32 resourceID);

void SEAddColorTarget(SERenderPipelineInfo* r, u32 idx);
void SEAddDrawFunc(SERenderPipelineInfo* r, SEDrawFunc f);

SERenderPipeline* SECompilePipeline(SEwindow* win, SERenderPipelineInfo* r);

void SEDestroyPipeline(SEwindow* win, SERenderPipeline* r);
void SEDestroyPipelineInfo(SEwindow* win, SERenderPipelineInfo* r);

void SEDrawPipeline(SEwindow* win, SERenderPipeline* r);

//descriptors
typedef enum SEBindingType {
    SE_BINDING_VERTEX,
    SE_BINDING_INSTANCE,
} SEBindingType;

void AddVertexBinding(SERenderPipelineInfo* rinfo, SEBindingType type, SEStructSpec* layout, u32 numMembers);

#endif
