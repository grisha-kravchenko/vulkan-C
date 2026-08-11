#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#define BUILD_SCRIPT
#include "headers/misc.h"

#define COMPILER "gcc"
#define TARGET "./target"

typedef enum {
    RUN = 1,
    DEBUG = 2,
    DEBUGGER = 4,
} FLAGS;

void print_help() {
    printf(
        "This is a basic build script for the program. Usage:\n"
        "    -r/--run   - run the program right after the compilation\n"
        "    -d/--debug - compile in debug mode\n"
        "    -g         - attach debugger\n"
        "    -h/--help  - print this help message\n"
    );
}

FLAGS get_flags(int argc, char** argv) {
    FLAGS ret = 0;

    while (argc > 1) {
        shift(argc, argv);
        if (strcmp(argv[0], "-h") == 0 || strcmp(argv[0], "-help") == 0) {
            print_help();
            exit(0);
        }
        else if (strcmp(argv[0], "-r") == 0) ret |= RUN;
        else if (strcmp(argv[0], "-run") == 0) ret |= RUN;
        else if (strcmp(argv[0], "-d") == 0) ret |= DEBUG;
        else if (strcmp(argv[0], "-debug") == 0) ret |= DEBUG;
        else if (strcmp(argv[0], "-g") == 0) ret |= DEBUGGER;
        else {
            printf("[ERROR] unknown flag: %s", argv[0]);
            print_help();
            exit(1);
        }
    }
    return ret;
}

int main(int argc, char **argv) {
    rebuild_builder(argv[0], "src/headers/misc.h");
    FLAGS flags = get_flags(argc, argv);

    char* cmd = NULL;
    cmd = cmd_append(cmd, COMPILER, "-o", TARGET);
    cmd = cmd_append(cmd, "src/init.c", "src/main.c", "src/vulkan_misc.c", "src/window.c");
    cmd = cmd_append(cmd, "-O3", "-lvulkan", "-lglfw", "-lm");

    if (flags && DEBUG) cmd = cmd_append(cmd, "-DDEBUG");
    if (flags && DEBUGGER) cmd = cmd_append(cmd, "-g");
    if (flags && RUN) cmd = cmd_append(cmd, "&&", TARGET);
    cmd_run(cmd);

    return 0;
}
