#ifndef P_MISC
#define P_MISC 1

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

typedef struct {
    size_t count;
    size_t capacity;
} ArrayHeader;

#define try(value, message) if (!(value))                           \
    {printf("[FATAL ERROR]: %s (%s:%i)\n", message, __FILE__, __LINE__); abort();}

#define try_c(value, message, callback) if (!(value))               \
    {printf("[FATAL ERROR]: %s (%s:%i)\n", message, __FILE__, __LINE__); callback(); abort();}

#define vec_len(vector) (((ArrayHeader *)(vector) - 1)->count)

#define vec_sized(array, size) do {                                                       \
    ArrayHeader* __header = malloc((size) * sizeof((array)[0]) + sizeof(ArrayHeader));    \
    try(__header, "Couldn't create vector from pointer");                                 \
    (array) = (void*)(__header + 1);                                                      \
    __header->capacity = (size);                                                          \
    __header->count = 0;                                                                  \
} while (0)

#define vec_push(vector, value) do {                                         \
    if ((vector) == NULL) vec_sized(vector, 32);                             \
    ArrayHeader* __header = (ArrayHeader*)(vector) - 1;                      \
                                                                             \
    if (++__header->count > __header->capacity) {                            \
        __header->capacity = __header->capacity * 1.5 + 32;                  \
        __header = realloc(                                                  \
            __header,                                                        \
            __header->capacity * sizeof(vector[0]) + sizeof(ArrayHeader)     \
        );                                                                   \
        try(__header, "Couldn't reallocate vector on push");                 \
        (vector) = (void*)(__header + 1);                                    \
    }                                                                        \
    vector[__header->count - 1] = (value);                                   \
} while (0)

#define vec_concat_array(vector, ptr, ptr_len) do {                              \
    if ((vector) == NULL) vec_sized((vector), 32);                               \
    ArrayHeader* __header1 = (ArrayHeader*)(vector) - 1;                         \
                                                                                 \
    if (__header1->count + (ptr_len) > __header1->capacity) {                    \
        __header1->capacity = (__header1->count + (ptr_len)) * 1.5 + 32;         \
        __header1 = realloc(                                                     \
            __header1,                                                           \
            __header1->capacity * sizeof((vector)[0]) + sizeof(ArrayHeader)      \
        );                                                                       \
        try(__header1, "Couldn't reallocate vector on concat");                  \
        (vector) = (void*)(__header1 + 1);                                       \
    }                                                                            \
                                                                                 \
    for (size_t __i_iterate = 0; __i_iterate < (ptr_len); ++__i_iterate) {       \
        vector[__header1->count + __i_iterate] = (ptr)[__i_iterate];             \
    }                                                                            \
    __header1->count = __header1->count + (ptr_len);                             \
} while (0)

#define vec_concat_str(vector, ptr) do {                   \
    if ((vector) == NULL) vec_sized(vector, 32);           \
    vec_concat_array((vector), (ptr), strlen(ptr) + 1);    \
    vec_len(vector)--;                                     \
} while (0)

#define vec_concat(vector, other) do {                     \
    if ((vector) == NULL) vec_sized(vector, 32);           \
    ArrayHeader* __header2 = (ArrayHeader*)(other) - 1;    \
    vec_concat_array(vector, other, __header2->count)      \
} while (0)

#define vec_pop(vector) do {                                \
    ArrayHeader* __header = (ArrayHeader*)(vector) - 1;     \
    if (__header->count <= 0) break;                        \
    __header->count--;                                      \
} while (0)

#define vec_free(vector) do {                              \
    if ((vector) == NULL) break;                           \
    ArrayHeader* __header = (ArrayHeader*)(vector) - 1;    \
    free(__header);                                        \
} while (0)

#define shift(ptr, count) do { try((count) > 0, "Called shift with less than 0 count"); (ptr)++; (count)--; } while (0)

#define new_vec(type, ...) (void*)((ArrayHeader *)(&(struct {    \
    size_t count;                                                \
    size_t capacity;                                             \
    type data[sizeof((type[]){__VA_ARGS__})/sizeof(type)];       \
}) {                                                             \
    .count = sizeof((type[]){__VA_ARGS__})/sizeof(type),         \
    .capacity = 0,                                               \
    __VA_ARGS__                                                  \
}) + 1)

static inline char* vec_to_str(char** vec) {
    size_t str_len = 1; // Null terminator
    for (size_t i = 0; i < vec_len(vec); ++i)
        str_len += strlen(vec[i]);

    char* string = malloc(str_len);
    string[0] = '\0';
    try(string, "Couln't allocate string")

    for (size_t i = 0; i < vec_len(vec); ++i)
        strcat(string, vec[i]);

    return string;
}


# ifdef BUILD_SCRIPT_IMPLEMENTATION
# define BUILD_SCRIPT
# endif

# ifdef BUILD_SCRIPT
# undef BUILD_SCRIPT

#define cmd_append(cmd, ...) cmd_append_counted(cmd, sizeof((const char*[]){__VA_ARGS__})/sizeof(const char*), (const char*[]){__VA_ARGS__})
#define cmd_execute(...) cmd_execute_counted(sizeof((const char*[]){__VA_ARGS__})/sizeof(const char*), (const char*[]){__VA_ARGS__})
#define rebuild_builder(argc, argv, ...) rebuild_builder_counted(argc, argv, sizeof((const char*[]){__FILE__, __VA_ARGS__})/sizeof(const char*), (const char*[]){__FILE__, __VA_ARGS__})

char* cmd_append_counted(char* cmd, size_t count, const char* args[]);
int cmd_run(char* cmd);
int cmd_execute_counted(size_t count, const char* cmds[]);
int compare_dates(char* file1, char* file2);
int cmd_run_conditional(char* cmd, char** sources, char** outputs);
void rebuild_builder_counted(int argc, char** argv, size_t sources_count, const char* sources[]);

# endif

# ifdef BUILD_SCRIPT_IMPLEMENTATION
# undef BUILD_SCRIPT_IMPLEMENTATION

#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

char* cmd_append_counted(char* cmd, size_t count, const char* args[]) {
    for (size_t i = 0; i < count; ++i) {
        vec_concat_str(cmd, args[i]);
        vec_concat_str(cmd, " ");
    }
    return cmd;
}

int cmd_run(char* cmd) {
    printf("[CMD]: %s\n", cmd);
    vec_len(cmd) = 0;
    return system(cmd);
}

int cmd_execute_counted(size_t count, const char* cmds[]) {
    char* cmd = NULL;
    cmd = cmd_append_counted(cmd, count, cmds);

    int ret_code = cmd_run(cmd);
    vec_free(cmd);
    return ret_code;
}

int compare_dates(char* file1, char* file2) {
    struct stat file1_stat;
    struct stat file2_stat;

    if (stat(file1, &file1_stat) == -1) {
        if (errno == ENOENT) return 1;
        perror("[ERROR]: Couldn't get stat of file 1");
        exit(1);
    }

    if (stat(file2, &file2_stat) == -1) {
        if (errno == ENOENT) return 1;
        perror("[ERROR]: Couldn't get stat of file 2");
        exit(1);
    }

    time_t file1_time = file1_stat.st_mtime;
    time_t file2_time = file2_stat.st_mtime;

    return file1_time < file2_time;
}

int cmd_run_conditional(char* cmd, char** sources, char** outputs) {
    if (vec_len(sources) == 0 || vec_len(outputs) == 0) return cmd_run(cmd);

    int should_run = 0;
    for (size_t i = 0; i < vec_len(outputs); ++i)
    for (size_t j = 0; j < vec_len(sources); ++j) {
        if (compare_dates(outputs[i], sources[j])) {
            should_run = 1;
            break;
        }
    };
    if (should_run == 0) {
        vec_len(cmd) = 0;
        return 0;
    }
    return cmd_run(cmd);
}

void rebuild_builder_counted(int argc, char** argv, size_t sources_count, const char* sources[]) {
    int should_run = 0;
    for (size_t i = 0; i < sources_count; ++i)
    if (compare_dates(argv[0], (char*)sources[i])) {
        should_run = 1;
        break;
    };

    if (should_run == 0) return;
    char* cmd = NULL;
    cmd = cmd_append(cmd, "gcc", "-o", argv[0], sources[0]);

    try(!cmd_run(cmd), "Couldn't compile build executable");

    cmd = cmd_append_counted(cmd, (size_t)argc, (const char**)argv);
    cmd_run(cmd);

    exit(0);
}

# endif

#endif
