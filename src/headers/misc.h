#ifndef P_MISC
#define P_MISC 1

#include "datatypes.h"

Vec vec_new(usize element_size);
Vec vec_from_array(usize element_size, void* ptr, usize count);
void* vec_get(Vec* vector, usize position);
i32 vec_set(Vec* vector, usize position, void* value);
i32 vec_push(Vec* vector, void* value);
i32 vec_push_str(Vec* vector, char* value);
void* vec_pop(Vec* vector);
void vec_free(Vec* vector);
char* vec_to_str(Vec* vector);

Iter vec_iter(Vec* vector);
void* iter_next(Iter* iter);

#endif
