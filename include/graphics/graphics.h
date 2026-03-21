#ifndef SE_GRAPHICS_H
#define SE_GRAPHICS_H

#include "cutils.h"
#include <core/introspect.h>

// TODO(ELI): Replace flags with engine provided types
// TODO(ELI): Custom logic stuff

typedef enum SEBufType {
    SE_BUFFER_VERT,
    SE_BUFFER_INDEX,
    SE_BUFFER_UNIFORM,
} SEBufType;

typedef enum SEMemType {
    SE_MEM_STATIC,
    SE_MEM_DYNAMIC,
} SEMemType;

typedef struct SEGraphicsSettings {
} SEGraphicsSettings;

typedef struct MemoryRange {
    u32 offset;
    u32 size;
} MemoryRange;

typedef enum SEBindingType {
    SE_BINDING_VERTEX,
    SE_BINDING_INSTANCE,
} SEBindingType;

typedef enum SEDescriptorType {
    SE_TEXTURE_2D,
    SE_UNIFORM_BUFFER,
} SEDescriptorType;

typedef enum SEShaderStage {
    SE_SHADER_VERTEX = 1 << 0,
    SE_SHADER_FRAGMENT = 1 << 1,
} SEShaderStage;

typedef struct SEwindow SEwindow;

// Rendergraph
typedef struct SERenderPipelineInfo SERenderPipelineInfo;
typedef struct SERenderPipeline SERenderPipeline;
typedef struct SECmdBuf SECmdBuf;

// rendercallback
typedef void (*SEDrawFunc)(SECmdBuf *p, void *pass);

#define SE_SCREEN 0

void DrawTriangle(SECmdBuf *buf, void *pass);

SERenderPipelineInfo *SECreateRenderPipeline(SEwindow *win);
void SEDestroyRenderPipelineInfo(SEwindow *win, SERenderPipelineInfo *r);

u32 SEAddColorAttachment(SEwindow *win, SERenderPipelineInfo *r);
u32 SENewPass(SEwindow *win, SERenderPipelineInfo *r);
u32 SEAddPipeline(SEwindow *win, SERenderPipelineInfo *r, u32 layout);
u32 SEAddVertexBuffer(SEwindow *win, SERenderPipelineInfo *r, SEMemType t, u32 size);
u32 SEAddUniformBuffer(SEwindow* win, SERenderPipelineInfo *r, SEMemType t, u32 size);

void SEUseVertexBuffer(SEwindow *win, SERenderPipelineInfo *r, u32 pass, u32 resourceID);
void SEWriteColorAttachment(SEwindow *win, SERenderPipelineInfo *r, u32 pass, u32 resourceID);
void SESetBackBuffer(SERenderPipelineInfo *r, u32 resourceID);
void SEUsePipeline(SERenderPipelineInfo *r, u32 pass, u32 pipe);

void SESetShaderVertex(SEwindow *win, SERenderPipelineInfo *info, u32 pipe, SString filename);
void SESetShaderFrag(SEwindow *win, SERenderPipelineInfo *info, u32 pipe, SString filename);
void SEAddVertexBinding(SERenderPipelineInfo *rinfo, u32 pass, SEBindingType type, SEStructSpec *layout, u32 numMembers);

u32 SEAddDescriptorLayout(SEwindow *win, SERenderPipelineInfo *r);
void SEAddDescriptorBinding(SEwindow *win, SERenderPipelineInfo *r, u32 layout, SEShaderStage stage, SEDescriptorType type, u32 count);

void SESetScissor(SERenderPipeline *p, u32 pass, f32 x, f32 y, f32 width, f32 height);
void SESetViewPort(SERenderPipeline *p, u32 pass, f32 x, f32 y, f32 width, f32 height);

SERenderPipeline *SECompilePipeline(SEwindow *win, SERenderPipelineInfo *info);
void SEExecutePipeline(SEwindow *win, SERenderPipeline *p);
void SEDestroyPipeline(SEwindow *win, SERenderPipeline *p);

void *SERetrieveDynBuf(SEwindow *win, SERenderPipeline *p, u32 buffer);
void SEUnmapDynBuf(SEwindow* win, SERenderPipeline *p, u32 buffer);
void SEBindUniformBuffer(SEwindow* win, SERenderPipeline* p, u32 pass, u32 buffer, u32 binding);

void SEUploadBuffer(SEwindow *win, SERenderPipeline *r, u32 resourceID, void *data, u32 size);
void SEDrawPipeline(SEwindow *win, SERenderPipeline *r);

#endif
