#ifndef SE_GRAPHICS_H
#define SE_GRAPHICS_H

#include <core/cutils.h>
#include <core/introspect.h>

// TODO(ELI): Replace flags with engine provided types
// TODO(ELI): Custom logic stuff

typedef enum SEBufType {
    SE_BUFFER_VERT,
    SE_BUFFER_INDEX,
    SE_BUFFER_UNIFORM,
} SEBufType;

typedef enum SEIndexType {
    SE_INDEX_U32 = 0, //anchor to zero
    SE_INDEX_U16,
} SEIndexType;

typedef enum SEImageFormat {
    SE_IMAGE_RGBA_32 = 0,
    SE_IMAGE_RGBA_8,
    SE_IMAGE_RGB_32,
    SE_IMAGE_RGB_8,
    SE_IMAGE_BGRA_8,
    SE_IMAGE_BGR_8,
    SE_IMAGE_FORMAT_NUM,
} SEImageFormat;

typedef enum SEMemType {
    SE_MEM_STATIC,
    SE_MEM_DYNAMIC,
} SEMemType;

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
u32 SEAddDepthAttachment(SEwindow* win, SERenderPipelineInfo *r);
u32 SENewPass(SEwindow *win, SERenderPipelineInfo *r);
u32 SEAddPipeline(SEwindow *win, SERenderPipelineInfo *r, u32 layout);
u32 SEAddVertexBuffer(SEwindow *win, SERenderPipelineInfo *r, SEMemType t, u32 size);
u32 SEAddIndexBuffer(SEwindow* win, SERenderPipelineInfo* r, SEMemType t, u32 size);
u32 SEAddUniformBuffer(SEwindow* win, SERenderPipelineInfo *r, SEMemType t, u32 size);
u32 SEAddTexture(SEwindow* win, SERenderPipelineInfo* r, SEImageFormat format, u32 width, u32 height);
u32 SEAddTextureSampler(SEwindow* win, SERenderPipelineInfo* r);

void SEUseVertexBuffer(SEwindow *win, SERenderPipelineInfo *r, u32 pass, u32 resourceID);
void SEUseIndexBuffer(SEwindow* win, SERenderPipelineInfo* r, u32 pass, u32 resourceID, SEIndexType index_type);
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
void SEBindTexture(SEwindow *win, SERenderPipeline *p, u32 pass, u32 tex, u32 sampler, u32 binding);

void SEUploadBuffer(SEwindow *win, SERenderPipeline *r, u32 resourceID, void *data, u32 size);
void SEUploadImage(SEwindow* win, SERenderPipeline* r, u32 resourceID, void* data);
void SEDrawPipeline(SEwindow *win, SERenderPipeline *r);

#endif
