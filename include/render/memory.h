#ifndef SE_RENDER_MEMORY_H
#define SE_RENDER_MEMORY_H

#include <util.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <render/render.h>

/*
 * The Plan v2:
 *
 * The plan now is to create a memory arena
 * for each kind of resource, ie: Static Textures,
 * GPU side read write textures, CPU writable textures,
 * Vertex Buffers, etc.
 * 
 *
 * Use nested arenas, one for each heap,
 * and one for each resource kind.
 *
 *
 */


typedef struct SE_render_heap {
    VkMemoryPropertyFlagBits props;
    u64 top;
    u64 size;
} SE_render_heap;

typedef struct SE_resource_arena {
    VkDeviceMemory devMem;
    void* resource;
    u64 size;
    u64 top;
    u64 alignment;
} SE_resource_arena;

typedef struct SE_render_memory {
    SE_render_heap* heaps;
    u32 numheaps;
} SE_render_memory;

typedef struct SE_render_buffer {
    u32 offset;
    u32 size;
} SE_render_buffer;

SE_render_memory SE_CreateHeapTrackers(SE_render_context* r);
SE_resource_arena SE_CreateResourceTrackerBuffer(SE_render_context* r, SE_render_memory* m, VkBufferUsageFlagBits flags, VkMemoryPropertyFlagBits props, u64 minsize);

#endif
