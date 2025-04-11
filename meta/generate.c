#include "generate.h"
#include "parser.h"
#include <stdio.h>

const char* baseTypes[] = {
    "char",
    "float",
    "Bool32",
    "wl_registry",
    "wl_display",
    "uint32_t",
    "uint64_t",
    "int32_t",
    "int64_t",
    "u32",
    "u64",
    "i32",
    "i64",
    "VkInstance",
    "VkSurfaceKHR",
    "VkDevice",
    "VkPhysicalDevice",
    "VkCommandPool",
    "VkCommandBuffer",
    "VkSwapchainKHR",
    "VkSurfaceFormatKHR",
    "VkPresentModeKHR",
    "VkExtent2D",
    "VkImage",
    "VkImageView",
    "VkQueue",
};

void GenerateStructDefHeader(FILE* out, StructData* head) {

    {
        StructData* curr = head;
        //enums
        fprintf(out, "typedef enum SE_meta_type{\n");
        for (int i = 0; i < sizeof(baseTypes)/sizeof(baseTypes[0]); i++) {
            fprintf(out, "\tMeta_Type_%s,\n", baseTypes[i]);
            fprintf(out, "\tMeta_Type_%s_pointer,\n", baseTypes[i]);
        }
        while (curr) {
            fprintf(out, "\tMeta_Type_%.*s,\n", curr->nameSize, curr->name);
            fprintf(out, "\tMeta_Type_%.*s_pointer,\n", curr->nameSize, curr->name);

            curr = curr->next;
        }
        fprintf(out, "} SE_meta_type;\n");
    }
}

void GenerateStructDefSource(FILE* out, StructData* head) {

    {
        StructData* curr = head;
        //Struct defs
        while (curr) {
            fprintf(out, "static SE_struct_member Meta_Def_%.*s[] = {\n", curr->nameSize, curr->name);

            for (int i = 0; i < curr->numMembers; i++) {
                StructMember m = curr->members[i];
                fprintf(out, "\t{Meta_Type_%.*s%.*s, \"%.*s\", (u64)&(((struct %.*s *)0)->%.*s), (u64)sizeof(((struct %.*s *)0)->%.*s)},\n", 
                        m.t.nameSize, m.t.basename, 
                        m.t.isPointer * 9, "_pointer",
                        m.nameSize, m.name,
                        curr->nameSize, curr->name,
                        m.nameSize, m.name,
                        curr->nameSize, curr->name,
                        m.nameSize, m.name
                      );
            }

            fprintf(out, "};\n");

            curr = curr->next;
        }
    }
}
