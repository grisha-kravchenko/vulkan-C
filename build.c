#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>

#define TARGET "./target"
#define BUILD_EXECUTABLE "./build"

typedef enum {
	COMPILE_EXECUTE = 1,
} CompilerFlags;

typedef struct {
	char* args;
	char** files;
	CompilerFlags flags;
	size_t count;
	char* argv;
} Context;

void print_help() {
	printf(
		"This is a basic build script for the program. Usage:\n"
		"	"__FILE__"   - just compile the program into "TARGET"\n"
		"	-r/--run  - run the program right after the compilation\n"
		"	-h/--help - print this help message\n"
	);
}

Context default_context(int argc, char** argv) {
	char* args = "-O3";
	CompilerFlags flags = 0;
	char* argv_array[(argc - 1) * 2];

	for (int i = 1; i < argc; ++i) {
		char* flag = argv[i];

		if (strcmp(flag, "-h") == 0 || strcmp(flag, "--help") == 0) {
			print_help();
			exit(0);
		} else if (strcmp(flag, "-r") == 0 || strcmp(flag, "--run") == 0) {
			flags |= COMPILE_EXECUTE;
		} else {
			fprintf(stderr, "[ERROR] unknown flag: %s\n", flag);
			print_help();
			exit(1);
		}

		argv_array[(i - 1) * 2] = " ";
		argv_array[(i - 1) * 2 + 1] = flag;
	}

	size_t argv_size = 1;
	for (int i = 0; i < (argc - 1) * 2; ++i) argv_size += strlen(argv_array[i]);

	char* argv_buffer = malloc(argv_size);
	if (argv_buffer == NULL) {
		perror("[ERROR] Couldn't allocate argv string buffer");
		exit(1);
	}
	argv_buffer[0] = '\0';
	for (int i = 0; i < (argc - 1) * 2; ++i) strcat(argv_buffer, argv_array[i]);

	Context ctx = {
		.args = args,
		.files = NULL,
		.flags = flags,
		.count = 0,
		.argv = argv_buffer,
	};

	return ctx;
}

bool should_rebuild_builder() {
	struct stat src_stat;
	struct stat dst_stat;

	if (stat(__FILE__, &src_stat) == -1) {
		perror("[ERROR] Couldn't get stat of "__FILE__);
		exit(1);
	}

	if (stat(BUILD_EXECUTABLE, &dst_stat) == -1) {
		if (errno == ENOENT) return true; // no executable found
		perror("[ERROR] Couldn't get stat of "BUILD_EXECUTABLE);
		exit(1);
	}

	time_t src_time = src_stat.st_mtime;
	time_t dst_time = dst_stat.st_mtime;

	return src_time > dst_time;
}

void rebuild_builder(Context* ctx) {
	if (!should_rebuild_builder()) return;
	printf("[INFO] Rebuilding the builder...\n");

	system("gcc "__FILE__" -o "BUILD_EXECUTABLE); // build the build script
	printf("[INFO] Compiled the builder executable: "BUILD_EXECUTABLE"\n");
	if (should_rebuild_builder()) {
		fprintf(stderr, "[ERROR] Need to rebuild executable after it was rebuilt\n");
		fprintf(stderr, "[ERROR] There is likely a problem inside compilation\n");
		exit(1);
	}

	if (strcmp(ctx->argv, "") != 0) {
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

void add_file(char* file, Context* ctx) {
	ctx->files = realloc(
		ctx->files,
		(++ctx->count) * sizeof(char*)
	);
	if (ctx->files == NULL) exit(2);
	ctx->files[ctx->count - 1] = file;
}

void compile(Context* ctx) {
	printf("[INFO] Compiling the project\n");
	// char* command_array[] = {"gcc ", file, " ", ctx->args, " -o ", file};
	size_t cmd_arr_len = ctx->count * 2 + 3;
	char* command_array[cmd_arr_len];
	command_array[0] = "gcc ";

	for (int idx = 0; idx < ctx->count; ++idx) {
		char* file = ctx->files[idx];
		printf("[INFO] Adding %s to compilation command\n", file);
		command_array[idx*2 + 1] = file;
		command_array[idx*2 + 2] = " ";
	}

	command_array[cmd_arr_len - 2] = ctx->args;
	command_array[cmd_arr_len - 1] = " -o "TARGET;

	size_t cmd_buf_len = 1; // null terminator
	for (int i = 0; i < cmd_arr_len; ++i) cmd_buf_len += strlen(command_array[i]);

	char command_buffer[cmd_buf_len];
	command_buffer[0] = '\0';
	for (int i = 0; i < cmd_arr_len; ++i) strcat(command_buffer, command_array[i]);

	printf("[INFO] Compiling: \"%s\"...\n", command_buffer);
	int compile_code = system(command_buffer);
	if (compile_code != 0) {
		fprintf(stderr, "[ERROR] Compiler exited with code: %i\n", compile_code);
		exit(1);
	}

	if (ctx->flags & COMPILE_EXECUTE != 0) {
		system(TARGET);
	}
}

int main(int argc, char **argv) {
	Context ctx = default_context(argc, argv);
	rebuild_builder(&ctx);

	printf("[INFO] Adding files to build\n");
	add_file("main.c", &ctx);

	compile(&ctx);
	return 0;
}
