#include <stdio.h>
#define SB_IMPL
#include "sb.h"



int main(int argc, char* argv[]) {
    AUTO_REBUILD(argc, argv);

    if (argc < 2) {
        sb_fprintf(stdin, "Platform spec Required");
        exit(-1);
    }

    sb_cmd* c = &(sb_cmd){0};

    FILE* f = fopen("compile_flags.txt", "w+");
    fprintf(f, "-Iinclude\n");
    fprintf(f, "-Ilib/vulkan/include\n");
    fprintf(f, "-pedantic\n");
    fprintf(f, "-std=c99\n");
    fprintf(f, "-Werror=vla\n");
    if (strcmp(argv[1], "wayland") == 0) {
        fprintf(f, "-D WAYLAND\n");
    } else if (strcmp(argv[1], "win32") == 0) {
        fprintf(f, "-D WIN32\n");
    }
    fprintf(f, "-DSE_DEBUG_CONSOLE\n");
    fprintf(f, "-DSE_ASSERT\n");
    fprintf(f, "-DSE_DEBUG_VULKAN\n");
    fclose(f); 
    

    sb_mkdir("build");
    sb_mkdir("build/shaders");

    //metaprogramming layer
    sb_pick_compiler();
    sb_cmd_push(c, sb_compiler(), "-g", "-Werror=vla"); 

    sb_cmd_push(c, "meta/meta.c");
    sb_cmd_push(c, "meta/token.c");
    sb_cmd_push(c, "meta/parser.c");
    sb_cmd_push(c, "meta/symboltable.c");
    sb_cmd_push(c, "meta/generate.c");

    sb_cmd_push(c, "meta/platform/src/platform.c");
    sb_cmd_push(c, sb_cmd_flag("Imeta/platform/include"));
    sb_cmd_push(c, "-o", "build/preprocessor");
    if (sb_cmd_sync_and_reset(c)) {
        return -1;
    }

    //run preprocessor

    sb_cmd_push(c, "./build/preprocessor");
    if (sb_cmd_sync_and_reset(c)) {
        return -1;
    }

    sb_cmd_push(c, "glslc", "shaders/test.glsl.vert", "-o", "build/shaders/vert.spv");
    if (sb_cmd_sync_and_reset(c)) {
        return -1;
    }

    sb_cmd_push(c, "glslc", "shaders/test.glsl.frag", "-o", "build/shaders/frag.spv");
    if (sb_cmd_sync_and_reset(c)) {
        return -1;
    }


    sb_cmd_push(c, sb_compiler(), "-std=c99", "-pedantic", "-g", "-Iinclude", "-Werror=vla");

    sb_cmd_push(c, "-Ilib/vulkan/include");
    sb_cmd_push(c, "-Llib/vulkan/Lib");
    sb_cmd_push(c, "-isystem");


    if (strcmp(argv[1], "wayland") == 0) {
        sb_cmd_push(c, "-c");
        sb_cmd_push(c, "src/math/math.c");
        sb_cmd_push(c, "src/util.c");
        sb_cmd_push(c, "src/render/vulkan.c");
        sb_cmd_push(c, "src/platform/WAYLAND/platform.c", "-D", "WAYLAND"); //TODO(ELI): Make switch with platform
        sb_cmd_push(c, "-lwayland-client");
        sb_cmd_push(c, "-lrt");
        sb_cmd_push(c, "-lc");
        sb_cmd_push(c, "-lvulkan");
    } else if (strcmp(argv[1], "win32") == 0) {
        sb_cmd_push(c, "src/math/math.c");
        sb_cmd_push(c, "src/util.c");
        sb_cmd_push(c, "src/render/vulkan.c");
        sb_cmd_push(c, "src/platform/WIN32/platform.c");
        sb_cmd_push(c, "-D", "WIN32");
        sb_cmd_push(c, "-luser32");
        sb_cmd_push(c, "-lvulkan-1");
        sb_cmd_push(c, "-Xlinker", "/subsystem:windows");
    }


    sb_cmd_push(c, "-D SE_DEBUG_CONSOLE");
    sb_cmd_push(c, "-D SE_ASSERT");
    sb_cmd_push(c, "-D SE_DEBUG_VULKAN");

    if (strcmp(argv[1], "wayland") == 0) {
        sb_cmd_push(c, "-o", "build/test");
    } else {
        sb_cmd_push(c, "-o", "build/test.exe");
    }
    
    if (sb_cmd_sync_and_reset(c)) {
        return -1;
    }

    sb_cmd_push(c, sb_compiler(), "-std=c99", "-pedantic", "-g", "-Iinclude", "-Werror=vla");
    if (strcmp(argv[1], "wayland") == 0) {
        sb_cmd_push(c, "-D", "WAYLAND");
    } else if (strcmp(argv[1], "win32") == 0) {
        sb_cmd_push(c, "-D", "WIN32");
    }
    sb_cmd_push(c, "tests/init.c");
    sb_cmd_push(c, "-Ilib/vulkan/include");
    sb_cmd_push(c, "-Llib/vulkan/Lib");
//    sb_cmd_push(c, "src/render/vulkan.c");
    sb_cmd_push(c, "-isystem");
    sb_cmd_push(c, "-fPIC");
    sb_cmd_push(c, "-shared");
    sb_cmd_push(c, "-o", "build/init.so");

    if (sb_cmd_sync_and_reset(c)) {
        return -1;
    }

    if (strcmp(argv[1], "wayland") == 0) {
        sb_cmd_push(c, "./build/test");
    } else {
        sb_cmd_push(c, "./build/test.exe");
    }

    //run test
    if (sb_cmd_sync_and_reset(c)) {
        return -1;
    }

    return 0;
}
