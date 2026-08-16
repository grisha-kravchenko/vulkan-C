#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#define BUILD_SCRIPT_IMPLEMENTATION
#include "headers/misc.h"

#define COMPILER "gcc"
#define TARGET "./target"

typedef enum {
    FLAG_RUN = 1,
    FLAG_DEBUG = 2,
    FLAG_DEBUGGER = 4,
    FLAG_RELEASE = 8,
} FLAGS;

void print_help() {
    printf(
        "This is a basic build script for the program.\nNote that it hot reloads on change.\nAviable flags:\n"
        "    -r/--run   - run the program right after the compilation\n"
        "    -d/--debug - compile in debug mode\n"
        "    --release  - enables all optimizations\n"
        "    -g         - attach debugger\n"
        "    -l         - print the dependencies list\n"
        "    -h/--help  - print this help message\n"
    );
}

FLAGS get_flags(int argc, char** argv) {
    FLAGS ret = 0;

    while (argc > 1) {
        shift(argv, argc);
        if (strcmp(argv[0], "-h") == 0 || strcmp(argv[0], "-help") == 0) {
            print_help();
            exit(0);
        }

        else if (strcmp(argv[0], "-r")        == 0) ret |= FLAG_RUN;
        else if (strcmp(argv[0], "--run")     == 0) ret |= FLAG_RUN;
        else if (strcmp(argv[0], "-d")        == 0) ret |= FLAG_DEBUG;
        else if (strcmp(argv[0], "--debug")   == 0) ret |= FLAG_DEBUG;
        else if (strcmp(argv[0], "-g")        == 0) ret |= FLAG_DEBUGGER;
        else if (strcmp(argv[0], "--release") == 0) ret |= FLAG_RELEASE;
        else if (strcmp(argv[0], "-l")        == 0) {
            printf(
                "[INFO]: Dependencies:\n"
                " - [Vulkan headers]             (https://github.com/KhronosGroup/vulkan-headers)\n"
                " - [glfw3]                      (https://github.com/glfw/glfw) (Compatible with wayland)\n"
                "[INFO]: Used libraries:\n"
                " - [Vulkan Memory Allocator]    (https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)\n"
            );
            exit(0);
        }

        else {
            printf("[ERROR]: unknown flag: %s", argv[0]);
            print_help();
            exit(1);
        }
    }
    return ret;
}

int main(int argc, char **argv) {
    rebuild_builder(argc, argv, "src/headers/misc.h");
    FLAGS flags = get_flags(argc, argv);

    char* cmd = NULL;

    struct stat st;
    if (stat("tmp", &st) == -1) mkdir("tmp", 0700);

    cmd = cmd_append(cmd, "gcc", "-o", "tmp/vma_impl.o", "-c", "src/vma_impl.cpp", "-O3", "&&");
    cmd = cmd_append(cmd, "gcc", "-r", "tmp/vma_impl.o", "-lstdc++", "-o", "tmp/vma.o");
    cmd_run_conditional(cmd, new_vec(char*, "src/vma_impl.cpp", "src/headers/thirdparty/vk_mem_alloc.h"), new_vec(char*, "tmp/vma.o"));

    cmd = cmd_append(cmd, "slangc", "-o", "tmp/test.spv", "--", "src/shaders/test.slang", "&&");
    cmd = cmd_append(cmd, "xxd", "-i", "tmp/test.spv", ">", "tmp/test.pv.h");
    if (cmd_run(cmd)) return 1;

    cmd = cmd_append(cmd, "gcc", "-o", TARGET);
    cmd = cmd_append(cmd, "src/init.c", "src/main.c", "src/vulkan_misc.c", "src/window.c", "tmp/vma.o");
    cmd = cmd_append(cmd, "-O3", "-lvulkan", "-lglfw", "-lm");

    if (flags & FLAG_DEBUG)    cmd = cmd_append(cmd, "-DDEBUG");
    if (flags & FLAG_DEBUGGER) cmd = cmd_append(cmd, "-g");
    if (flags & FLAG_RELEASE)  cmd = cmd_append(cmd, "-flto", "-fdata-sections", "-ffunction-sections", "-Wl,--gc-sections", "-s");
    cmd_run(cmd);
    if (flags & FLAG_RUN) {
        cmd = cmd_append(cmd, TARGET);
        cmd_run(cmd);
    }

    vec_free(cmd);
    return 0;
}
