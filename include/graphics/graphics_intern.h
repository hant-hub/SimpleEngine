#ifndef SE_GRAPHICS_INTERN_H
#define SE_GRAPHICS_INTERN_H

#include "cutils.h"
#include "se.h"
#include "ds.h"
#include "vulkan/vk_platform.h"
#include "vulkan/vulkan_core.h"


#include <graphics/graphics.h>

PoolDecl(ShaderPool, VkShaderModule);
PoolDecl(LayoutPool, VkPipelineLayout);
PoolDecl(PipelinePool, VkPipeline);



//shouldn't be static since this should be shared across
//platforms

typedef struct SwapChainInfo {
    VkSurfaceCapabilitiesKHR cap;
    VkSurfaceFormatKHR* formats;
    VkPresentModeKHR* modes;
    u32 numformats;
    u32 nummodes;
} SwapChainInfo;

typedef struct MemoryManager {
    dynArray(MemoryRange) freelist;
} MemoryManager;

typedef struct SEBuffer {
    u32 parent;
    SEBufType type;
    MemoryRange r;
} SEBuffer;

typedef struct BufferAllocator {
    VkBuffer b;
    u32 memid;
    u64 alignment;
    MemoryRange r; //range of memory allocation
    MemoryManager m;
} BufferAllocator;
BufferAllocator InitBufferAllocator(SEwindow* w, VkBufferCreateInfo info, u32 props);
BufferAllocator SEConfigBufType(SEwindow* w, SEBufType bt, SEMemType mt, u64 size);
SEBuffer AllocBuffer(SEwindow* win, BufferAllocator* allocator, u32 bufID, u64 size);

typedef struct SEImage {
    VkImage img;
    VkImageView view;
    MemoryRange r;
    VkFormat format;
    VkImageLayout layout;
    u32 memid;
    u32 width, height;
} SEImage;


//vulkan
typedef struct SEVulkan {
    //graphics
    //NOTE(ELI): If I ever decide to support
    //multiwindow applications, the instance, physical device
    //and possibly the logical device need to be split into
    //a global
    VkInstance inst;
    VkPhysicalDevice pdev;
    SwapChainInfo swapInfo;
    VkDevice dev; 
    VkSurfaceKHR surf;
    VkCommandPool pool;
    VkCommandBuffer buf;

    dynArray(VkSemaphore) imgAvalible;
    dynArray(VkSemaphore) renderfinished;
    VkFence inFlight;

    struct {
        dynArray(MemoryManager) heaps;
        dynArray(VkDeviceMemory) mem;
        dynArray(u32) types;
        dynArray(VkMemoryPropertyFlags) props;
    } memory;

    //dynArray(BufferAllocator) bufAllocators;

    struct {
        VkCommandPool pool;
        VkCommandBuffer cmd;
        VkDeviceMemory mem;
        VkBuffer buf;
        void* ptr;
    } transfer;

    struct {
        //Will be the same if
        //same queue supports both operations.
        u32 gfam;
        u32 pfam;
        u32 tfam;
        VkQueue graphics;
        VkQueue present;
        VkQueue transfer;
    } queues;

    struct {
        //NOTE(ELI): stored in context since there
        //will never be more than one swapchain per
        //window
        VkSurfaceFormatKHR format;
        VkPresentModeKHR mode;
        u32 width;
        u32 height;
        u32 imgcount;
        VkSwapchainKHR swap;

        VkImage* imgs;
        VkImageView* views;
    } swapchain;

    struct {
        ShaderPool shaders;
        LayoutPool layouts;
        PipelinePool pipelines;
    } resources;

    #ifdef DEBUG
    VkDebugUtilsMessengerEXT debugMessenger;
    #endif
} SEVulkan;

void SEConfigMaxGPUMem(SEwindow* win, SEMemType t, u64 size);

//Rendergraph data
typedef struct SEAttachmentInfo {
    VkFormat f;
    bool8 clear;
    bool8 persistant; //used to determine whether image layout
                      //is undefined at start of each frame
} SEAttachmentInfo;

typedef enum ResourceUsage {
    RESOURCE_UNINITIALIZED = 0,
    RESOURCE_READ  = 1 << 0,
    RESOURCE_WRITE = 1 << 1,
} ResourceUsage;

typedef enum ResourceType {
    RESOURCE_COLOR_ATTACHMENT = 0,
    RESOURCE_TEXTURE,
    RESOURCE_VERTEX_BUFFER,
} ResourceType;

typedef struct Resource {
    ResourceType type;
    ResourceUsage lastUsage; 
    
    //TODO(ELI): move views etc into a central
    //Image Allocator. Use Resource Type + idx
    //to retrieve resource info.
    u32 idx;
    union {
        VkImageView view;
        u32 bufIdx;
    } vk;

    bool8 clear;
    bool8 swapchain;
} Resource;

typedef struct BufferInfo {
    SEMemType memType;
    u32 size;
} BufferInfo;

//structure to store graph before object creation
typedef struct SERenderPassInfo {
    u32 readStart;
    u32 numReads;
    u32 writeStart;
    u32 numWrites;

    struct {
        u32 vertex;
        u32 fragment;
        u32 layout;
    } pipeline;

    struct {
        u32 firstBinding;
        u32 numBindings;

        u32 firstAttr;
        u32 numAttrs;
    } vertInfo;

    void* func;
} SERenderPassInfo;

typedef struct SERenderPipelineInfo {
    Allocator a;

    dynArray(VkVertexInputBindingDescription) bindings;
    dynArray(VkVertexInputAttributeDescription) attrs;
    dynArray(SERenderPassInfo) passes;
    dynArray(VkShaderModule) shaders;
    dynArray(VkPipelineLayout) layouts;
    dynArray(u32) writes;
    dynArray(u32) reads;
    dynArray(Resource) resources;
    dynArray(BufferInfo) vertBufInfo;
    dynArray(BufferInfo) indexBufInfo;
} SERenderPipelineInfo;

typedef struct SECmdBuf {
    VkCommandBuffer buf;
} SECmdBuf;

typedef struct SEPass {
    VkRenderPass rpass;
    VkPipeline pipe;
    SEDrawFunc func;
    u32 framebuffer;

    struct {
        u32 start;
        u32 size;
    } vertbufs;

    struct {
        u32 start;
        u32 size;
    } idxbufs;

    bool8 targetSwap; //If we target a swapchain image
                      //We need to use the image index to
                      //pick the correct framebuffer
} SEPass;

typedef struct PipelineBarrier {
    VkPipelineStageFlags srcStageMask; 
    VkPipelineStageFlags dstStageMask;
    VkDependencyFlags depFlags; //specify region stuff (probably ignore)
    VkMemoryBarrier barrier;
    VkImageMemoryBarrier imageBarrier; //do layout transitions (probably not necessary)
    VkBufferMemoryBarrier bufferBarrier;
} PipelineBarrier;

typedef struct SERenderPipeline {
    Allocator a;

    dynArray(Resource) resourceMaps;
    dynArray(VkCommandBuffer) buf;

    dynArray(PipelineBarrier) barriers;
    dynArray(VkFramebuffer) framebuffers;

    dynArray(BufferAllocator) bufAllocators;

    dynArray(SEBuffer) vertexBuffers;
    dynArray(SEBuffer) indexBuffers;

    dynArray(SEPass) passes;
} SERenderPipeline;

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData); 


extern VKAPI_ATTR VkResult VKAPI_CALL (*CreateDebugMessenger)(
    VkInstance                                  instance,
    const VkDebugUtilsMessengerCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugUtilsMessengerEXT*                   pMessenger);

extern VKAPI_ATTR void VKAPI_CALL (*DestroyDebugMessenger)(
    VkInstance                                  instance,
    VkDebugUtilsMessengerEXT                    messenger,
    const VkAllocationCallbacks*                pAllocator);

MemoryManager CreateManager(Allocator a, u32 size);
MemoryRange AllocateDeviceMem(MemoryManager* m, u32 size, u32 alignment);
void FreeDeviceMem(MemoryManager* m, MemoryRange r);
void DestroyManager(MemoryManager m);


void LoadExtensionFuncs(VkInstance instance);

void CreateVulkan(VkInstance inst, VkSurfaceKHR surf, SEwindow* win, SEVulkan* g, SEsettings* s);
void DestroyVulkan(SEVulkan g, Allocator a);

//helpers
VkPipeline CreatePipeline(SEVulkan* v, VkRenderPass r,
        VkVertexInputBindingDescription* bindings,
        u32 bindnum,
        VkVertexInputAttributeDescription* attributes,
        u32 attrnum,
        VkPipelineLayout layout,
        VkShaderModule vert,
        VkShaderModule frag);

#endif
