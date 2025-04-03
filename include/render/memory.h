#ifndef SE_RENDER_MEMORY_H
#define SE_RENDER_MEMORY_H

#include <util.h>
#include <vulkan/vulkan.h>

/*
 * The Plan:
 *
 * Search device memory types,
 * for each type create a SE_render_memory
 *
 * Each SE_render_memory is an arena,
 * with a large block of GPU memory allocated
 * and then buffers being suballocated
 *
 * For now the plan is to put these memory arenas
 * into SE_render_context
 *
 * If they run out of memory, crash the program.
 *
 * For the final version, use a metaprogramming tool
 * to scan each allocation and calculate the required
 * size.
 *
 *
 * If necessary create a heap allocater for the future
 *
 *
 * For right now allocate each buffer individually,
 * maybe in future use arenas on individual types of
 * buffers, ie: vertex arena, index arena, etc.
 */


typedef struct SE_render_memory {
    VkDeviceMemory devMem;
    u64 top;
    u64 size;
} SE_render_memory;


#endif
