#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

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
                " - [Lua Language]               (https://github.com/lua/lua)\n"
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

// ignoring the existence of make
char* build_lua(char* cmd, FLAGS flags) {
    struct stat st;
    if (stat("tmp/lua", &st) == -1) mkdir("tmp/lua", 0700);

    DIR* lua = opendir("vendor/lua");
    try(lua, "couldn't open ./vendor/lua folder");
    struct dirent* lua_file;

    char** object_files = NULL;

    while ((lua_file = readdir(lua)) != NULL) {
        char* file_name = lua_file->d_name;
        size_t name_len = strlen(file_name);

        if (file_name[name_len - 2] != '.' || file_name[name_len - 1] != 'c')
            continue;
        if (strcmp(file_name, "lua.c") == 0 || strcmp(file_name, "luac.c") == 0)
            continue;

        char* output_file = vec_to_str(new_vec(char*, "tmp/lua/", file_name, ".o"));
        char* source_file = vec_to_str(new_vec(char*, "vendor/lua/", file_name));

        cmd = cmd_append(cmd, "gcc", "-c", "-o", output_file, source_file, "-O3", "-Wall", "-Wextra");
        cmd_run_conditional(cmd, new_vec(char*, source_file), new_vec(char*, output_file));

        free(source_file);
        vec_push(object_files, output_file);
    }

    closedir(lua);

    cmd = cmd_append(cmd, "ar", "rcus", "tmp/liblua.a");
    for (size_t i = 0; i < vec_len(object_files); ++i) cmd = cmd_append(cmd, object_files[i]);
    cmd_run_conditional(cmd, new_vec(char*, "vendor/lua/lua.c"), new_vec(char*, "tmp/liblua.a"));

    for (size_t i = 0; i < vec_len(object_files); ++i) free(object_files[i]);
    vec_free(object_files);

    return cmd;
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

    cmd = build_lua(cmd, flags);

    cmd = cmd_append(cmd, "slangc", "-o", "tmp/test.spv", "--", "src/shaders/test.slang", "&&");
    cmd = cmd_append(cmd, "xxd", "-i", "tmp/test.spv", ">", "tmp/test.pv.h");
    if (cmd_run(cmd)) return 1;

    cmd = cmd_append(cmd, "gcc", "-o", TARGET);
    cmd = cmd_append(cmd, "src/init.c", "src/main.c", "src/vulkan_misc.c", "src/window.c", "tmp/vma.o");
    cmd = cmd_append(cmd, "-O3", "-lvulkan", "-lglfw", "-lm");
    cmd = cmd_append(cmd, "-I./vendor/lua", "tmp/liblua.a");

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
