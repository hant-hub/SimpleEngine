#ifndef SE_GRAPHICS_H
#define SE_GRAPHICS_H

#include "cutils.h"
#include <core/introspect.h>

//TODO(ELI): Replace flags with engine provided types
//TODO(ELI): Custom logic stuff

typedef enum SEBufType {
    SE_BUFFER_VERT,
    SE_BUFFER_INDEX,
} SEBufType;

typedef enum SEImgFormat {
    SE_IMG_RGB_F32,
    SE_IMG_RGB_I8,
} SEImgFormat;

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

typedef struct SEBuffer {
    u32 parent;
    SEBufType type;
    MemoryRange r;
} SEBuffer;

typedef struct SEImage {
    u32 parent;
    SEImgFormat format;
    MemoryRange r;
} SEImage;

typedef struct SEDynBuf {
    SEBuffer b;
    void* mem;
} SEDynBuf;

typedef struct SEStaticBuf {
    SEBuffer b;
} SEStaticBuf;

typedef struct SEwindow SEwindow;

//Rendergraph
typedef struct SERenderPipelineInfo SERenderPipelineInfo;
typedef struct SERenderPipeline SERenderPipeline;
typedef struct SECmdBuf SECmdBuf;

//rendercallback
typedef void (*SEDrawFunc)(SECmdBuf* p, void* pass);

#define SE_SCREEN 0

void DrawTriangle(SECmdBuf* buf, void* pass);

void* GetHandle(SEwindow* win, SEBuffer b);
void FreeHandle(SEwindow* win, SEBuffer b, void* ptr);
void CPUtoGPUBufferMemcpy(SEwindow* win, SEBuffer dst, void* src, u32 size);

SERenderPipelineInfo* SECreatePipeline(SEwindow* win);
void SEBeginRenderPass(SERenderPipelineInfo* r);
void SEEndRenderPass(SERenderPipelineInfo* r);

u32 SEAddVertexBuffer(SERenderPipelineInfo* r, SEMemType type, u32 size);

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
