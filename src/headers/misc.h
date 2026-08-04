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

#define vec_len(vector) ((ArrayHeader *)(vector) - 1)->count

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
    if ((vector) == NULL) vec_sized((vector), 32);         \
    vec_concat_array((vector), (ptr), strlen(ptr) + 1);    \
    vec_len(vector)--;                                     \
} while (0)

#define vec_concat(vector, other) do {                     \
    if ((other) == NULL) vec_sized(other, 32);             \
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

#endif
