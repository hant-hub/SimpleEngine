#include "cutils.h"
#include "se.h"
#include "vulkan/vulkan_core.h"
#include <graphics/graphics_intern.h>


SEShader AddShader(SEwindow* v, SString source) {
    SEVulkan* g = GetGraphics(v);

    file src = fileopen(source, FILE_READ);
    
    SString code = {
        .len = src.stats.size,
        .data = Alloc(v->mem, src.stats.size)
    };

    u64 read = fileread(code, src);
    fileclose(src);

    VkShaderModuleCreateInfo shaderInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = read,
        .pCode = (u32*)code.data,
    };

    u32 new = ShaderPoolAlloc(&g->resources.shaders);
    vkCreateShaderModule(g->dev, &shaderInfo, NULL, &g->resources.shaders.slots[new]);
    Free(v->mem, code.data, code.len);

    return (SEShader)new;
}
