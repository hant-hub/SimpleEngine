#include <stdio.h>
#include <ctype.h>
#define SB_IMPL
#include "sb.h"


void CpyUpper(char* out, const char* in);

int main(int argc, char* argv[]) {
    AUTO_REBUILD(argc, argv);

    //read platform
    if (argc < 2) {
        printf("Please Specify Platform!\n");
        exit(1);
    }
    const char* platform = argv[1];

    char src_macro_name[256] = {0};
    char src_platform_path[256] = {0};
    CpyUpper(src_macro_name, platform);
    snprintf(src_platform_path, 256, "src/platform/%s/%s.c", platform, platform);

    sb_cmd* c = &(sb_cmd){0};
    sb_cmd_push(c, "mkdir", "-p", "build");
    if (sb_cmd_sync(c)) {
        exit(1);
    }
    sb_cmd_clear_args(c);

    sb_cmd_push(c, "mkdir", "-p", "build/shaders");
    if (sb_cmd_sync(c)) {
        exit(1);
    }
    sb_cmd_clear_args(c);

    //create compile_flags.txt
    FILE* f = fopen("compile_flags.txt", "w+");
    fprintf(f, "-Iinclude\n");
    fprintf(f, "-DSE_DEBUG_LOG\n");

    if (strcmp(platform, "wayland") == 0) {
        fprintf(f, "-DWAYLAND");
        fprintf(f, "-lwayland-client\n");
    }

    fclose(f);

    //TODO(ELI): For now we will create an executable for
    //each test, in future will move to an executable engine,
    //and a application DLL

    //TODO(ELI): add command options to customize the tests
    //run

    sb_cmd_push(c, "cc", "-g", "-D", platform);
    sb_cmd_push(c, "-DSE_DEBUG_LOG");
    sb_cmd_push(c, "-Iinclude");
    sb_cmd_push(c, "-lvulkan");
    sb_cmd_push(c, "src/tests/window.c");
    sb_cmd_push(c, "src/vulkan.c");
    sb_cmd_push(c, "src/utils.c");

    if (strcmp(platform, "wayland") == 0) {
        sb_cmd_push(c, "-lwayland-client");
    }

    sb_cmd_push(c, src_platform_path);
    sb_cmd_push(c, "-o", "build/test");

    if (sb_cmd_sync(c)) {
        exit(1);
    }
    sb_cmd_clear_args(c);
    
    sb_cmd_push(c, "glslc", "shaders/test.glsl.vert", "-o", "build/shaders/vert.spv");
    if (sb_cmd_sync(c)) {
        exit(1);
    }
    sb_cmd_clear_args(c);

    sb_cmd_push(c, "glslc", "shaders/test.glsl.frag", "-o", "build/shaders/frag.spv");
    if (sb_cmd_sync(c)) {
        exit(1);
    }
    sb_cmd_clear_args(c);

    sb_cmd_push(c, "./build/test");
    if (sb_cmd_sync(c)) {
        exit(1);
    }

    return 0;
}

void CpyUpper(char* out, const char* in) {
    while (*in) {
        *out = toupper(*(in++));
    }
}
