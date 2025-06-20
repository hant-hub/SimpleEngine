#ifndef INTERNAL_H
#define INTERNAL_H

#include "funcdefs.h"

/*
 * This file is for all the internal only information,
 * essentially only internal function definitions.
 *
 * The data structues exist in the dedicated header files,
 * Maybe at some point they will be combined into a single file.
 *
 *
 */

//Function Definitions ---------------------------------------


/* -------------------------------------+
 *      Pure Render Functions           |
 *                                      |
 * -------------------------------------*/

//Core Functions ---------------------------------

//Heap allocates a scratch arena
SE_CreateRenderContextFunc(SE_CreateRenderContext);

//Either pass in a preallocated memory arena
//Or pass Null and an arena will be automatically
//allocated on the Heap for scratch work
SE_CreateSwapChainFunc(SE_CreateSwapChain);

//GPU Memory Functions ------------------------------
SE_CreateHeapTrackersFunc(SE_CreateHeapTrackers);
SE_TransferMemoryFunc(SE_TransferMemory);

SE_CreateResourceTrackerBufferFunc(SE_CreateResourceTrackerBuffer);
SE_CreateBufferFunc(SE_CreateBuffer);

SE_CreateResourceTrackerRawFunc(SE_CreateResourceTrackerRaw);
SE_CreateRawFunc(SE_CreateRaw);

//Pipeline Cache Functions -------------------------------
SE_InitPipelineCacheFunc(SE_InitPipelineCache);
SE_PushPipelineTypeFunc(SE_PushPipelineType);
SE_FreePipelineCacheFunc(SE_FreePipelineCache);

//Pipeline Functions ---------------------------------
SE_LoadShaderFunc(SE_LoadShaders);
SE_CreateSyncObjsFunc(SE_CreateSyncObjs);

//Pipeline Creation
SE_BeginPipelineCreationFunc(SE_BeginPipelineCreation);
SE_EndPipelineCreationFunc(SE_EndPipelineCreation);

//Add resource
SE_AddShaderFunc(SE_AddShader);
SE_AddVertSpecFunc(SE_AddVertSpec);
SE_AddDepthAttachmentFunc(SE_AddDepthAttachment);

//Passes
SE_OpqaueNoDepthPassFunc(SE_OpqaueNoDepthPass);
SE_OpaquePassFunc(SE_OpqauePass);

//Resize and Draw
SE_CreateFrameBuffersFunc(SE_CreateFrameBuffers);
SE_DrawFrameFunc(SE_DrawFrame);

//Vertex Functions ---------------------------------------
SE_CreateVertSpecInlineFunc(SE_CreateVertSpecInline);


#endif
