#ifndef SE_GRAPHICS_INTERN_H
#define SE_GRAPHICS_INTERN_H

#include "core/cutils.h"
#include "ds/ds.h"
#include "se.h"
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
    u32 memid;
    u32 width, height;
} SEImage;
SEImage AllocImage(SEVulkan* v, VkImageUsageFlags usage, VkFormat format, u32 width, u32 height);


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
        VkCommandPool pool;
        VkCommandBuffer cmd;
    } graphics;

    struct {
        VkCommandPool pool;
        VkCommandBuffer cmd;
    } present;

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

//Rendergraph

typedef enum Resourcetype {
    RESOURCE_IMAGE,
    RESOURCE_BUFFER,
} Resourcetype;

typedef enum AccessFlag {
    ACCESS_UNINITIALIZED = 0,
    ACCESS_PREINITIALIZED,
    ACCESS_COLOR_ATTACHMENT,
} AccessFlag;

typedef struct Resource {
    Resourcetype type;

    union {
        struct SEImageInfo {
            AccessFlag lastAccess;
            f32 width, height;
            VkFormat format;
            VkImageUsageFlags usage;
            bool8 swapRel;
            bool8 persist;
            VkImageLayout layout;
        } img;

        struct SEBufferInfo {
            VkBufferUsageFlags usage;
            SEMemType memType;
            u32 size;
        } buf;
    } resourceInfo;
} Resource;

typedef struct PipelineInfo {
    u32 layout;

    VkPipelineInputAssemblyStateCreateInfo    pInputAssemblyState;
    VkPipelineTessellationStateCreateInfo     pTessellationState;
    VkPipelineViewportStateCreateInfo         pViewportState;
    VkPipelineRasterizationStateCreateInfo    pRasterizationState;
    VkPipelineMultisampleStateCreateInfo      pMultisampleState;
    VkPipelineDepthStencilStateCreateInfo     pDepthStencilState;
    VkPipelineColorBlendStateCreateInfo       pColorBlendState;
    VkPipelineDynamicStateCreateInfo          pDynamicState;

    VkPipelineColorBlendAttachmentState colorBlend;
    VkDynamicState dynStates[2];

    dynArray(VkVertexInputBindingDescription) bindings;
    dynArray(VkVertexInputAttributeDescription) attrs;
    dynArray(u32) DescriptorSets; 
    VkShaderModule vert;
    VkShaderModule frag;
} PipelineInfo;

typedef struct DescriptorLayout {
    u32 sets_required;
    VkPipelineLayout layout;
    VkDescriptorSetLayout desclayout;
    VkDescriptorSetLayoutCreateInfo info;
    dynArray(VkDescriptorSetLayoutBinding) bindings;
} DescriptorLayout;

typedef struct PassInfo {
    dynArray(u32) color_attachments; 
    dynArray(u32) vertex_buffers;
    u32 pipeline;
} PassInfo;

typedef struct SERenderPipelineInfo {
    u32 backbuffer;
    dynArray(Resource) resources;
    dynArray(PassInfo) passes;
    dynArray(PipelineInfo) pipeline;
    dynArray(DescriptorLayout) layouts;
} SERenderPipelineInfo;

typedef struct Layout {
    VkDescriptorPool pool;
} Layout;

typedef struct Pass {
    struct {
        VkPipeline pipeline;
        VkPipelineLayout pipeLayout;
        VkRenderPass pass;
        u32 framebuffer;
        u32 layout;
        VkDescriptorSet set;
    } pass;

    struct {
        struct {
            u32 start;
            u32 num;
        } verts;
    } resources;

    VkViewport view;
    struct {
        f32 x;
        f32 y;
        f32 width;
        f32 height;
    } scissor;

    v2u size;

    void (*draw)();
    bool8 (*clear)();
} Pass;

typedef struct FrameBufferInfo {
    u32 first;
    u32 num;
} FrameBufferInfo;

typedef enum BufAllocType {
    BUF_ALLOC_VERT_STATIC = 0,
    BUF_ALLOC_VERT_DYN,
    BUF_ALLOC_UNIFORM_STATIC,
    BUF_ALLOC_UNIFORM_DYN,
    BUF_ALLOC_NUM,
    BUF_ALLOC_INVALID = ~0,
} BufAllocType;

typedef struct SERenderPipeline {
    u32 backbuffer; // image to blit to swapchain img

    VkCommandBuffer* cmdBufs;

    dynArray(BufferAllocator) bufAllocators;
    dynArray(Pass) passes;
    dynArray(Layout) layouts;

    dynArray(u32) resourceMapping; //maps resourceID to resource ie: vertBuffer or image
    dynArray(u32) passVertMapping;
    dynArray(SEBuffer) buffers;

    dynArray(FrameBufferInfo) framebufferInfos;
    dynArray(VkFramebuffer) framebuffers;
    dynArray(u32) frameBufferMapping;

    //used for resizing
    dynArray(struct SEImageInfo) imgInfos;
    dynArray(SEImage) images;
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
VkPipeline CreatePipeline(SEVulkan* v, VkRenderPass r, PipelineInfo* info, VkPipelineLayout layout);
VkPipelineLayout CreatePipelineLayout(SEVulkan* v, DescriptorLayout* l);
Layout CompileLayout(SEVulkan* v, DescriptorLayout* layout);
VkShaderModule CompileShader(SEVulkan* v, SString data);

void CreateSwapChain(SEwindow* win, SEVulkan* g, Allocator a);

//Memory Helpers
void CPUtoGPUBufferMemcpy(SEwindow* win, BufferAllocator* a, SEBuffer* dst, void* src, u32 size);

#endif
