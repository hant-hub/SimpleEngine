#define SB_IMPL
#include "sb.h"

int main(int argc, char* argv[]) {
    AUTO_REBUILD(argc, argv);

    sb_cmd* c = &(sb_cmd){0};
    sb_cmd_push(c, "mkdir", "-p", "build");

    return 0;
}
