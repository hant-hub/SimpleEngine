#include <stdio.h>
#define SB_IMPL
#include "sb.h"

typedef enum Platform { X11, WAYLAND, WIN32, NONE } Platform;

Platform p = NONE;

int main(int argc, char *argv[]) {
    sb_BUILD(argc, argv) {
        sb_chdir_exe();
        sb_mkdir("build/");
        sb_mkdir("build/tests");
        sb_mkdir("build/objs");
        sb_target_dir("build/");

        if (sb_check_arg("x11")) {
            p = X11;
        } else if (sb_check_arg("wayland")) {
            p = WAYLAND;
        } else if (sb_check_arg("win32")) {
            p = WIN32;
        } else {
            printf("No Backend Specified\n");
            exit(1);
        }

        if (sb_check_arg("init")) {
            // downloads latest version of
            // cutils
            sb_CMD() {
                sb_cmd_main("curl");
                sb_cmd_arg(
                    "https://raw.githubusercontent.com/hant-hub/Cutils/refs/"
                    "heads/main/include/cutils.h");
                sb_cmd_opt("O");
                sb_cmd_opt("-output-dir");
                sb_cmd_arg("lib/include/");
            }
        }

        // formatting
        if (sb_check_arg("format")) {
            sb_FOREACHFILE("./", test) {
                if (sb_cmpext(test, ".c") && sb_cmpext(test, ".h"))
                    continue;

                sb_CMD() {
                    sb_cmd_main("clang-format");
                    sb_cmd_opt("i");
                    sb_cmd_arg(test);
                }
            }
            sb_FOREACHFILE("test/", test) {
                if (sb_cmpext(test, ".c") && sb_cmpext(test, ".h"))
                    continue;

                sb_CMD() {
                    sb_cmd_main("clang-format");
                    sb_cmd_opt("i");
                    sb_cmd_arg(test);
                }
            }
            sb_FOREACHFILE("include/", test) {
                if (sb_cmpext(test, ".c") && sb_cmpext(test, ".h"))
                    continue;

                sb_CMD() {
                    sb_cmd_main("clang-format");
                    sb_cmd_opt("i");
                    sb_cmd_arg(test);
                }
            }
            sb_FOREACHFILE("./", test) {
                if (sb_cmpext(test, ".c") && sb_cmpext(test, ".h"))
                    continue;

                sb_CMD() {
                    sb_cmd_main("clang-format");
                    sb_cmd_opt("i");
                    sb_cmd_arg(test);
                }
            }
        }
        sb_fence();

        char *windowdir = NULL;
        switch (p) {
            case X11: windowdir = "src/x11/"; break;
            case WAYLAND: windowdir = "src/wayland/"; break;
            case WIN32: windowdir = "src/win32/"; break;
            case NONE: break;
        }

        sb_FOREACHFILE(windowdir, source) {
            sb_EXEC() {
                sb_add_file(source);


                sb_add_include_path("include/");
                sb_add_include_path("lib/include");

                sb_link_library("vulkan");

                sb_add_flag("g");

                switch (p) {
                    case X11: {
                        sb_add_flag("DX11"); 
                        sb_link_library("X11");
                    } break;
                    case WAYLAND: sb_add_flag("DWAYLAND"); break;
                    case WIN32: sb_add_flag("DWIN32"); break;
                    case NONE: break;
                }

                char buf[PATH_MAX + 1] = {0};
                char final[PATH_MAX + 1] = {0};
                strncpy(buf, source, PATH_MAX);

                char *name = sb_stripext(sb_basename(buf));
                snprintf(final, PATH_MAX, "objs/%s.o", name);

                sb_add_flag("c");
                sb_set_out(final);

                sb_export_command();
            }
        }

        sb_FOREACHFILE("build/objs/", file) {
            sb_CMD() {
                sb_cmd_main("ar");
                sb_cmd_arg("rcs");
                sb_cmd_arg("build/libse.a");
                sb_cmd_arg(file);
            }
        }

        sb_fence();

        // tests
        sb_FOREACHFILE("test/tests/", test) {
            printf("test: %s\n", test);
            if (sb_cmpext(test, ".c"))
                continue;
            sb_EXEC() {
                sb_add_file(test);
                sb_add_file("build/libse.a");

                sb_add_include_path("include/");
                sb_add_include_path("lib/include");

                sb_link_library("vulkan");

                sb_add_flag("g");
                sb_add_flag("fsanitize=address");
                sb_link_library("m");

                switch (p) {
                    case X11: {
                        sb_add_flag("DX11"); 
                        sb_link_library("X11");
                    } break;
                    case WAYLAND: sb_add_flag("DWAYLAND"); break;
                    case WIN32: sb_add_flag("DWIN32"); break;
                    case NONE: break;
                }

                char buf[PATH_MAX + 1] = {0};
                char final[PATH_MAX + 1] = {0};
                strncpy(buf, test, PATH_MAX);

                char *name = sb_stripext(sb_basename(buf));
                snprintf(final, PATH_MAX, "tests/%s", name);

                sb_set_out(final);

                sb_export_command();
            }
        }

        sb_EXEC() {
            sb_add_file("test/runner.c");

            sb_add_include_path("include/");
            sb_add_include_path("lib/include");

            switch (p) {
                case X11: sb_add_flag("DX11"); break;
                case WAYLAND: sb_add_flag("DWAYLAND"); break;
                case WIN32: sb_add_flag("DWIN32"); break;
                case NONE: break;
            }

            sb_add_flag("g");
            sb_link_library("m");

            sb_set_out("runner");

            sb_export_command();
        }
    }

    if (!sb_check_arg("no-test")) {
        sb_build_start(argc, argv);
        sb_target_dir("build/");
        sb_CMD() { sb_cmd_main("clear"); }
        sb_fence();
        sb_CMD() {
            sb_cmd_main("build/runner");
            if (sb_check_arg("v")) {
                sb_cmd_arg("v");
            }
        }
        sb_build_end();
    }
}
