#include <render/render.h>
#include <generated/generated_static.h>
#include <generated/generated_def.h>

static SE_struct_member Meta_Def_SE_render_context[] = {
	{Meta_Type_VkInstance, "instance", (u64)&(((struct SE_render_context *)0)->instance)},
	{Meta_Type_VkSurfaceKHR, "surf", (u64)&(((struct SE_render_context *)0)->surf)},
	{Meta_Type_VkDevice, "l", (u64)&(((struct SE_render_context *)0)->l)},
	{Meta_Type_VkPhysicalDevice, "p", (u64)&(((struct SE_render_context *)0)->p)},
	{Meta_Type_VkCommandPool, "pool", (u64)&(((struct SE_render_context *)0)->pool)},
	{Meta_Type_VkCommandBuffer, "cmd", (u64)&(((struct SE_render_context *)0)->cmd)},
	{Meta_Type_Queues, "Queues", (u64)&(((struct SE_render_context *)0)->Queues)},
	{Meta_Type_Queues, "Queues", (u64)&(((struct SE_render_context *)0)->Queues)},
	{Meta_Type_SE_swapchain, "s", (u64)&(((struct SE_render_context *)0)->s)},
};
static SE_struct_member Meta_Def_Queues[] = {
	{Meta_Type_u32, "gfam", (u64)&(((struct Queues *)0)->gfam)},
	{Meta_Type_u32, "pfam", (u64)&(((struct Queues *)0)->pfam)},
	{Meta_Type_VkQueue, "g", (u64)&(((struct Queues *)0)->g)},
	{Meta_Type_VkQueue, "p", (u64)&(((struct Queues *)0)->p)},
};
static SE_struct_member Meta_Def_SE_swapchain[] = {
	{Meta_Type_VkSwapchainKHR, "swap", (u64)&(((struct SE_swapchain *)0)->swap)},
	{Meta_Type_VkSurfaceFormatKHR, "format", (u64)&(((struct SE_swapchain *)0)->format)},
	{Meta_Type_VkPresentModeKHR, "mode", (u64)&(((struct SE_swapchain *)0)->mode)},
	{Meta_Type_VkExtent2D, "size", (u64)&(((struct SE_swapchain *)0)->size)},
	{Meta_Type_u32, "numImgs", (u64)&(((struct SE_swapchain *)0)->numImgs)},
	{Meta_Type_VkImage_pointer, "imgs", (u64)&(((struct SE_swapchain *)0)->imgs)},
	{Meta_Type_VkImageView_pointer, "views", (u64)&(((struct SE_swapchain *)0)->views)},
};
