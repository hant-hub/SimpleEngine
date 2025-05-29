#define SB_IMPL
#include "sb.h"


int main(int argc, char* argv[]) {
    sb_BUILD(argc, argv) {
        sb_target_dir("build/");
        sb_chdir_exe();
        sb_mkdir("build");

        sb_EXEC() {
            sb_set_out("app");

            sb_add_flag("g");

            sb_add_flag("Iinclude");
            sb_add_flag("Ilib/vulkan/include");

            sb_add_flag("DSE_DEBUG_CONSOLE");
            sb_add_flag("DSE_ASSERT");
            sb_add_flag("DSE_DEBUG_VULKAN");

            sb_add_flag("pedantic");
            sb_add_flag("Werror=vla");

            sb_link_library("vulkan");
            sb_link_library("wayland-client");


            if (sb_check_arg("wayland")) {
                sb_add_file("src/platform/WAYLAND/platform.c");
                sb_add_flag("DWAYLAND");
            } else if (sb_check_arg("win32")) {
                sb_add_file("src/platform/WIN32/platform.c");
            }

            sb_add_file("src/render/vulkan.c");
            sb_add_file("src/util.c");
            sb_add_file("src/math/math.c");

            sb_export_command();
        }
        sb_EXEC() {
            sb_add_flag("Iinclude");
            sb_add_flag("Ilib/vulkan/include");

            sb_set_out("init.so");
            sb_add_flag("shared");

            sb_add_file("tests/init.c");

            if (sb_check_arg("wayland")) {
                sb_add_flag("DWAYLAND");
            } else if (sb_check_arg("win32")) {
            }

            sb_export_command();
        }
        sb_EXEC() {
            sb_set_out("preprocessor");

            sb_add_flag("Imeta/platform/include");

            sb_add_file("meta/meta.c");
            sb_add_file("meta/generate.c");
            sb_add_file("meta/parser.c");
            sb_add_file("meta/symboltable.c");
            sb_add_file("meta/token.c");

            sb_add_file("meta/platform/src/platform.c");

            sb_export_command();
        }

        sb_mkdir("build/shaders");
        sb_CMD() {
            sb_cmd_main("glslc");
            sb_cmd_arg("shaders/test.glsl.vert");
            sb_cmd_opt("obuild/shaders/vert.spv");
        }

        sb_CMD() {
            sb_cmd_main("glslc");
            sb_cmd_arg("shaders/test.glsl.frag");
            sb_cmd_opt("obuild/shaders/frag.spv");
        }

        sb_fence();
        sb_CMD() {
            sb_cmd_main("./build/app");
        }
    }
}
