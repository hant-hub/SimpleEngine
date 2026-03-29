#include <stdio.h>
#include <string.h>
#define SB_IMPL
#include "sb.h"

#define ARGS X(platform, string)

#define OPTS                                                                                                           \
    X(i, bool)                                                                                                         \
    X(f, bool)                                                                                                         \
    X(v, bool)                                                                                                         \
    X(build, string)                                                                                                   \
    X(t, string)                                                                                                       \
    X(mem, bool)

#include "sa.h"

typedef enum Platform { X11, WAYLAND, WIN32, NONE } Platform;
typedef enum Build { DEBUG, RELEASE } Build;

Platform p = NONE;
Build b = DEBUG;

int main(int argc, char *argv[]) {

    SA args = Parse(argc, argv);
    if (args.err) {
        sb_printf("Failed to parse!\n");
        return -1;
    }
    SAvals parsed = args.vals;

    if (sb_strcmp("x11", parsed.platform) == 0) {
        p = X11;
    } else if (sb_strcmp("wayland", parsed.platform) == 0) {
        p = WAYLAND;
    } else if (sb_strcmp("win32", parsed.platform) == 0) {
        p = WIN32;
    } else {
        printf("No Backend Specified\n");
        return -1;
    }

    if (parsed.build.set) {
        if (sb_strcmp("debug", parsed.build.val) == 0) {
            b = DEBUG;
        } else if (sb_strcmp("release", parsed.build.val) == 0) {
            b = RELEASE;
        }
    } else
        b = DEBUG;

    sb_BUILD(argc, argv) {
        sb_chdir_exe();
        sb_mkdir("build/");
        sb_mkdir("build/tests");
        sb_mkdir("build/objs");
        sb_target_dir("build/");

        if (parsed.i.set) {
            // downloads latest version of
            // cutils
            sb_CMD() {
                sb_cmd_main("git");
                sb_cmd_arg("clone");
                sb_cmd_arg("git@github.com:hant-hub/Cutils.git");
                sb_cmd_arg("lib/cutils");
            }
        }

        // formatting
        if (parsed.f.set) {
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

        char *buildflag = NULL;
        switch (b) {
            case DEBUG: buildflag = "DDEBUG"; break;
            case RELEASE: buildflag = "DRELEASE"; break;
        }

        sb_FOREACHFILE(windowdir, source) {
            sb_EXEC() {
                sb_add_file(source);

                sb_add_include_path("include/");
                sb_add_include_path("lib/cutils/include/");

                sb_link_library("vulkan");

                sb_add_flag("g");
                sb_add_flag(buildflag);
                if (parsed.mem.set)
                    sb_add_flag("fsanitize=address");

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
                snprintf(final, PATH_MAX, "objs/plat_%s.o", name);

                sb_add_flag("c");
                sb_set_out(final);

                sb_export_command();
            }
        }

        sb_FOREACHFILE("src/core/", source) {
            sb_EXEC() {
                sb_add_file(source);

                sb_add_include_path("include/");
                sb_add_include_path("lib/cutils/include/");

                sb_add_flag("g");
                sb_add_flag(buildflag);
                if (parsed.mem.set)
                    sb_add_flag("fsanitize=address");

                char buf[PATH_MAX + 1] = {0};
                char final[PATH_MAX + 1] = {0};
                strncpy(buf, source, PATH_MAX);

                char *name = sb_stripext(sb_basename(buf));
                snprintf(final, PATH_MAX, "objs/core_%s.o", name);

                sb_add_flag("c");
                sb_set_out(final);

                sb_export_command();
            }
            sb_fence();
        }

        sb_FOREACHFILE("src/graphics/", source) {
            sb_EXEC() {
                sb_add_file(source);

                sb_add_include_path("include/");
                sb_add_include_path("lib/cutils/include/");

                sb_add_flag("g");
                sb_add_flag(buildflag);
                if (parsed.mem.set)
                    sb_add_flag("fsanitize=address");

                char buf[PATH_MAX + 1] = {0};
                char final[PATH_MAX + 1] = {0};
                strncpy(buf, source, PATH_MAX);

                char *name = sb_stripext(sb_basename(buf));
                snprintf(final, PATH_MAX, "objs/graphics_%s.o", name);

                sb_add_flag("c");
                sb_set_out(final);

                sb_export_command();
            }
            sb_fence();
        }

        char* source = "lib/cutils/src/cutils.c";
        sb_EXEC() {
            sb_add_file(source);

            sb_add_include_path("include/");
            sb_add_include_path("lib/cutils/include/");

            sb_add_flag("g");
            sb_add_flag(buildflag);
            if (parsed.mem.set)
                sb_add_flag("fsanitize=address");

            char buf[PATH_MAX + 1] = {0};
            char final[PATH_MAX + 1] = {0};
            strncpy(buf, source, PATH_MAX);

            char *name = sb_stripext(sb_basename(buf));
            snprintf(final, PATH_MAX, "objs/core_%s.o", name);

            sb_add_flag("c");
            sb_set_out(final);

            sb_export_command();
        }
        sb_fence();

        // Static lib
        sb_FOREACHFILE("build/objs/", file) {
            sb_CMD() {
                sb_cmd_main("ar");
                sb_cmd_opt("rcs");
                sb_cmd_arg("build/libse.a");
                sb_cmd_arg(file);
            }
            // this is here to prevent ar from smashing the archive
            sb_fence();
        }

        // tests
        sb_FOREACHFILE("test/tests/", test) {
            printf("test: %s\n", test);
            if (sb_cmpext(test, ".c"))
                continue;
            sb_EXEC() {
                sb_add_file(test);
                sb_add_file("build/libse.a");

                sb_add_include_path("include/");
                sb_add_include_path("lib/cutils/include/");

                sb_link_library("vulkan");

                sb_add_flag("g");
                sb_add_flag(buildflag);
                sb_link_library("m");

                if (parsed.mem.set)
                    sb_add_flag("fsanitize=address");

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

        // compile shaders to spv
        sb_mkdir("build/shaders");
        sb_FOREACHFILE("shaders/", shader) {
            sb_CMD() {
                sb_cmd_main("glslc");
                sb_cmd_arg(shader);
                sb_cmd_opt("o");

                char buf[PATH_MAX + 1] = {0};
                char final[PATH_MAX + 1] = {0};
                strncpy(buf, shader, PATH_MAX);
                snprintf(final, PATH_MAX, "build/shaders/%s.spv", sb_basename(shader));

                sb_cmd_arg(final);

                printf("shader: %s\n", final);
            }
        }
        
        sb_CMD() {
            sb_cmd_main("cp");
            sb_cmd_opt("r");
            sb_cmd_arg("assets");
            sb_cmd_arg("build/");
        }

        sb_EXEC() {
            sb_add_file("test/runner.c");
            sb_add_file("lib/cutils/src/cutils.c");

            sb_add_include_path("include/");
            sb_add_include_path("lib/cutils/include/");

            switch (p) {
                case X11: sb_add_flag("DX11"); break;
                case WAYLAND: sb_add_flag("DWAYLAND"); break;
                case WIN32: sb_add_flag("DWIN32"); break;
                case NONE: break;
            }

            sb_add_flag("g");
            sb_add_flag(buildflag);
            sb_link_library("m");

            sb_set_out("runner");

            sb_export_command();
        }
    }
    if (!parsed.t.set)
        return 0;

    if (sb_strcmp("all", parsed.t.val) == 0) {
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
    } else {
        sb_build_start(argc, argv);
        sb_target_dir("build/");
        sb_CMD() { sb_cmd_main("clear"); }
        sb_fence();
        char buf[PATH_MAX] = {0};
        sb_snprintf(buf, PATH_MAX, "build/tests/%s", parsed.t.val);
        sb_CMD() { sb_cmd_main(buf); }
        sb_build_end();
    }
}
