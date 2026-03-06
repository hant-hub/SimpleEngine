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

typedef struct SEwindow SEwindow;

//Rendergraph
typedef struct SERenderPipelineInfo SERenderPipelineInfo;
typedef struct SERenderPipeline SERenderPipeline;
typedef struct SECmdBuf SECmdBuf;

//rendercallback
typedef void (*SEDrawFunc)(SECmdBuf* p, void* pass);

#define SE_SCREEN 0

void DrawTriangle(SECmdBuf* buf, void* pass);

SERenderPipelineInfo* SECreateRenderPipeline(SEwindow* win);

u32 SEAddColorAttachment(SEwindow* win, SERenderPipelineInfo* r);
u32 SENewPass(SEwindow* win, SERenderPipelineInfo* r);
u32 SEAddPipeline(SEwindow *win, SERenderPipelineInfo *r);

void SEWriteColorAttachment(SEwindow* win, SERenderPipelineInfo* r, u32 pass, u32 resourceID);
void SESetBackBuffer(SERenderPipelineInfo* r, u32 resourceID);
void SEUsePipeline(SERenderPipelineInfo* r, u32 pass, u32 pipe);

void SESetShaderVertex(SEwindow* win, SERenderPipelineInfo* info, u32 pipe, SString filename);
void SESetShaderFrag(SEwindow* win, SERenderPipelineInfo* info, u32 pipe, SString filename);

SERenderPipeline *SECompilePipeline(SEwindow *win, SERenderPipelineInfo *info);

void* SEMapVertBuffer(SEwindow* win, SERenderPipeline* r, u32 resourceID);
void SEUnMapVertBuffer(SEwindow* win, SERenderPipeline* r, u32 resourceID);

void SEUploadBuffer(SEwindow* win, SERenderPipeline* r, u32 resourceID, void* data, u32 size);
void SEDrawPipeline(SEwindow* win, SERenderPipeline* r);

//descriptors
typedef enum SEBindingType {
    SE_BINDING_VERTEX,
    SE_BINDING_INSTANCE,
} SEBindingType;

#endif
