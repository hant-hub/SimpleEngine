#include <stdio.h>
#define SB_IMPL
#include "sb.h"



int main(int argc, char* argv[]) {
    AUTO_REBUILD(argc, argv);

    sb_cmd* c = &(sb_cmd){0};

    FILE* f = fopen("compile_flags.txt", "w+");
    fprintf(f, "-Iinclude\n");
    fprintf(f, "-pedantic\n");
    fprintf(f, "-std=c99\n");
    fprintf(f, "-Werror=vla\n");
    fprintf(f, "-D WAYLAND\n");
    fprintf(f, "-D SE_DEBUG_CONSOLE\n");
    fprintf(f, "-D SE_ASSERT\n");
    fprintf(f, "-D SE_DEBUG_VULKAN\n");
    fclose(f); 
    

    sb_cmd_push(c, "mkdir", "-p", "build");
    if (sb_cmd_sync_and_reset(c)) {
        return -1;
    }

    sb_cmd_push(c, "mkdir", "-p", "build/shaders");
    if (sb_cmd_sync_and_reset(c)) {
        return -1;
    }

    //metaprogramming layer
    sb_cmd_push(c, "cc", "-g", "-Werror=vla"); 
    sb_cmd_push(c, "meta/meta.c");
    sb_cmd_push(c, "meta/tokenizer.c");
    sb_cmd_push(c, "meta/parser.c");
    sb_cmd_push(c, "meta/generate.c");
    sb_cmd_push(c, "-o", "build/preprocessor");
    if (sb_cmd_sync_and_reset(c)) {
        return -1;
    }

    //run preprocessor

    sb_cmd_push(c, "./build/preprocessor");
    if (sb_cmd_sync_and_reset(c)) {
        return -1;
    }
    //return 0; // temporary for testing

    sb_cmd_push(c, "glslc", "shaders/test.glsl.vert", "-o", "build/shaders/vert.spv");
    if (sb_cmd_sync_and_reset(c)) {
        return -1;
    }

    sb_cmd_push(c, "glslc", "shaders/test.glsl.frag", "-o", "build/shaders/frag.spv");
    if (sb_cmd_sync_and_reset(c)) {
        return -1;
    }


    sb_cmd_push(c, "cc", "-std=c99", "-pedantic", "-g", "-Iinclude", "-Werror=vla", "-D WAYLAND");
    sb_cmd_push(c, "-D SE_DEBUG_CONSOLE");
    sb_cmd_push(c, "-D SE_ASSERT");
    sb_cmd_push(c, "-D SE_DEBUG_VULKAN");
    sb_cmd_push(c, "src/platform/WAYLAND/platform.c"); //TODO(ELI): Make switch with platform
    sb_cmd_push(c, "src/render/vulkan.c");
    sb_cmd_push(c, "src/util.c");

    sb_cmd_push(c, "-lwayland-client");
    sb_cmd_push(c, "-lrt");
    sb_cmd_push(c, "-lc");
    sb_cmd_push(c, "-lvulkan");
    sb_cmd_push(c, "-o", "build/test");

    if (sb_cmd_sync_and_reset(c)) {
        return -1;
    }

    sb_cmd_push(c, "./build/test");

    //run test
    if (sb_cmd_sync_and_reset(c)) {
        return -1;
    }

    return 0;
}
