#include "vulkan/vk_platform.h"
#include "vulkan/vulkan_core.h"
#include "cutils.h"

typedef struct SEwindow SEwindow;
typedef struct SERenderPass SERenderPass;

typedef u32 SEShader;
typedef u32 SELayout;
typedef u32 SEPipeline;

SEShader AddShader(SEwindow* v, SString source);
u32 SEAddLayout(SEwindow* win);
u32 SEAddPipeline(SEwindow* win, SERenderPass* r, u32 layout, u32 vert, u32 frag);

SERenderPass* SECreateRenderPass(SEwindow* win);
void SEDestroyRenderPass(SEwindow* win, SERenderPass* r);

void BeginRender(SEwindow* win);
void EndRender(SEwindow* win, u32 idx);

u32 BeginRenderPass(SEwindow* win, SERenderPass* r);
void EndRenderPass(SEwindow* win);

void DrawTriangle(SEwindow* win);
