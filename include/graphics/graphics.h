#include "vulkan/vk_platform.h"
#include "vulkan/vulkan_core.h"
#include "cutils.h"

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
