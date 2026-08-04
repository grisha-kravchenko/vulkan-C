#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>

#include "headers/misc.h"

#define SOURCE "./src/"
#define COMPILER "gcc"
#define TARGET "./target"
#define BUILD_EXECUTABLE "./build"

typedef enum {
    COMPILE_EXECUTE = 1,
} CompilerFlags;

typedef struct {
    char* args;
    char** files;
    char** headers;
    char* argv;
    CompilerFlags flags;
} Context;

void print_help() {
    printf(
        "This is a basic build script for the program. Usage:\n"
        "    "BUILD_EXECUTABLE"   - just compile the program into "TARGET"\n"
        "    -r/--run   - run the program right after the compilation\n"
        "    -d/--debug - compile in debug mode\n"
        "    -g         - attach debugger\n"
        "    -h/--help  - print this help message\n"
    );
}

Context default_context(int argc, char** argv) {
    char* args = NULL;
    vec_concat_str(args, "-lvulkan -lglfw -lm -O3 ");

    CompilerFlags flags = 0;
    char* argv_str = NULL;

    while (argc > 1) {
        shift(argv, argc);
        char* flag = argv[0];

        if (strcmp(flag, "-h") == 0 || strcmp(flag, "--help") == 0) {
            print_help();
            exit(0);
        } else if (strcmp(flag, "-r") == 0 || strcmp(flag, "--run") == 0) {
            flags |= COMPILE_EXECUTE;
        } else if (strcmp(flag, "-d") == 0 || strcmp(flag, "--debug") == 0) {
            vec_concat_str(args, "-DDEBUG ");
        } else if (strcmp(flag, "-g") == 0) {
            vec_concat_str(args, "-g ");
        } else {
            fprintf(stderr, "[ERROR] unknown flag: %s\n", flag);
            print_help();
            exit(1);
        }

        vec_concat_str(argv_str, " ");
        vec_concat_str(argv_str, flag);
    }

    Context ctx = {
        .args = args,
        .files = NULL,
        .headers = NULL,
        .flags = flags,
        .argv = argv_str,
    };

    return ctx;
}

void free_ctx(Context* ctx) {
    vec_free(ctx->headers);
    vec_free(ctx->files);
    vec_free(ctx->argv);
    vec_free(ctx->args);
}

int should_rebuild_builder(Context* ctx) {
    struct stat src_stat;
    struct stat dst_stat;

    if (stat(BUILD_EXECUTABLE, &dst_stat) == -1) {
        if (errno == ENOENT) return 1; // no executable found
        perror("[ERROR] Couldn't get stat of "BUILD_EXECUTABLE);
        exit(1);
    }

    time_t dst_time = dst_stat.st_mtime;

    char** source = ctx->files;
    size_t build_sources_len = vec_len(ctx->files);
    while (build_sources_len > 0) {
        if (stat(*source, &src_stat) == -1) {
            char* err = NULL;
            vec_concat_str(err, "[ERROR] Couldn't get stat of ");
            vec_concat_str(err, *source);
            perror(err);
            exit(1);
        }

        time_t src_time = src_stat.st_mtime;
        if (src_time > dst_time) return 1;

        shift(source, build_sources_len);
    }
    if (ctx->headers == NULL) return 0;

    source = ctx->headers;
    build_sources_len = vec_len(ctx->headers);
    while (build_sources_len > 0) {
        if (stat(*source, &src_stat) == -1) {
            char* err = NULL;
            vec_concat_str(err, "[ERROR] Couldn't get stat of ");
            vec_concat_str(err, *source);
            perror(err);
            exit(1);
        }

        time_t src_time = src_stat.st_mtime;
        if (src_time > dst_time) return 1;

        shift(source, build_sources_len);
    }

    return 0;
}

void rebuild_builder(Context* ctx) {
    if (!should_rebuild_builder(ctx)) return;
    printf("[INFO] Rebuilding the builder...\n");

    char* cmd = NULL;
    vec_concat_str(cmd, COMPILER" ");

    char** sources = ctx->files;
    size_t sources_len = vec_len(ctx->files);
    while (sources_len > 0) {
        vec_concat_str(cmd, *sources);
        vec_concat_str(cmd, " ");

        shift(sources, sources_len);
    }
    vec_concat_str(cmd, "-o "BUILD_EXECUTABLE);

    printf("[INFO] Running \"%s\"\n", cmd);
    system(cmd); // build the build script
    vec_free(cmd);
    printf("[INFO] Compiled the builder executable: "BUILD_EXECUTABLE"\n");

    if (should_rebuild_builder(ctx)) {
        fprintf(stderr, "[ERROR] Need to rebuild executable after it was rebuilt\n");
        fprintf(stderr, "[ERROR] There is likely a problem inside compilation\n");
        exit(1);
    }

    if (ctx->argv != NULL) {
        int executable_len = strlen(BUILD_EXECUTABLE) + strlen(ctx->argv) + 1;
        char command[executable_len];
        command[0] = '\0';
        strcat(command, BUILD_EXECUTABLE);
        strcat(command, ctx->argv);

        system(command);
    } else system(BUILD_EXECUTABLE);

    printf("[INFO] Exiting\n");
    exit(0);
}


void compile(Context* ctx) {
    printf("[INFO] Compiling the project\n");
    char* cmd = NULL;
    vec_concat_str(cmd, COMPILER" ");

    char** files = ctx->files;
    size_t files_len = vec_len(files);

    while (files_len > 0) {
        printf("[INFO] Adding %s to compilation command\n", *files);
        vec_concat_str(cmd, *files);
        vec_concat_str(cmd, " ");
        shift(files, files_len);
    }

    vec_concat_str(cmd, ctx->args);
    vec_concat_str(cmd, " -o "TARGET);

    printf("[INFO] Compiling: \"%s\"...\n-------------------------------\n", cmd);
    int compile_code = system(cmd);
    vec_free(cmd);

    if (compile_code != 0) {
        fprintf(stderr, "\n[ERROR] Compiler exited with code: %i\n", compile_code);
        exit(1);
    }

    if ((ctx->flags & COMPILE_EXECUTE) != 0) {
        printf("\n");
        system(TARGET);
        printf("\n");
    }
}

int main(int argc, char **argv) {
    Context ctx = default_context(argc, argv);
    Context builder_ctx = default_context(argc, argv);

    vec_push(builder_ctx.files, SOURCE "build.c");
    vec_push(builder_ctx.headers, SOURCE "headers/misc.h");

    rebuild_builder(&builder_ctx);
    free_ctx(&builder_ctx);

    // printf("%s\n", builder_sources.values);
    printf("[INFO] Adding files to build\n");

    vec_push(ctx.files, SOURCE "init.c");
    vec_push(ctx.files, SOURCE "main.c");
    vec_push(ctx.files, SOURCE "vulkan_misc.c");
    vec_push(ctx.files, SOURCE "window.c");
    printf("[INFO] Added files to build\n");

    compile(&ctx);
    free_ctx(&ctx);
    return 0;
}
