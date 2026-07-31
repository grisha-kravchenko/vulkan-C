#include "headers/datatypes.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Vec vec_new(usize element_size) {
	Vec vector = {0, 0, NULL, element_size};
	return vector;
};

Vec vec_from_array(usize element_size, void* ptr, usize count) {
	Vec vector = {count, count, ptr, element_size};
	return vector;
}

void* vec_get(Vec* vector, usize position) {
	if (position >= vector->count) return NULL;
	return vector->values + position * vector->sizeof_member;
};

i32 vec_set(Vec* vector, usize position, void* value) {
	if (position >= vector->count) return -1;
	for (usize i = 0; i < vector->sizeof_member; ++i) {
		((char*)vector->values)[position * vector->sizeof_member + i] =
			((char*)value)[i];
	}
	return 0;
};

i32 vec_push(Vec* vector, void* value) {
	if (++vector->count > vector->capacity) {
		vector->capacity = vector->capacity * 1.5 + 32;
		vector->values = realloc(
			vector->values,
			vector->capacity * vector->sizeof_member
		);
		if (vector->values == NULL) return -1;
	}
	return vec_set(vector, vector->count - 1, value);
};

i32 vec_push_str(Vec* vector, char* str) {
	assert(vector->sizeof_member == sizeof(char*) && "[FATAL ERROR] Vector must be of type 'char*'");
	return vec_push(vector, &str);
}

void* vec_pop(Vec* vector) {
	if (vector->count == 0) return NULL;
	return vector->values + (--vector->count) * vector->sizeof_member;
};

void vec_free(Vec* vector) {
	free(vector->values);
	vector->values = NULL;
	vector->capacity = 0;
	vector->count = 0;
};

char* vec_to_str(Vec* vector) {
	assert(vector->sizeof_member == sizeof(char*) && "[FATAL ERROR] Vector must be of type 'char*'");
	usize string_length = 1; // null terminator
	for (usize i = 0; i < vector->count; ++i)
		string_length += strlen(*(char**)(vector->values + i * vector->sizeof_member));

	char* output_string = malloc(string_length);
	if (output_string == NULL) return NULL;
	output_string[0] = '\0';

	for (usize i = 0; i < vector->count; ++i)
		strcat(output_string, *(char**)(vector->values + i * vector->sizeof_member));

	return output_string;
}

Iter vec_iter(Vec* vector) {
	Iter iter = { vector->count, vector->values - vector->sizeof_member, vector->sizeof_member };
	return iter;
};

void* iter_next(Iter* iter) {
	if (iter->count == 0) return NULL;
	iter->count--;
	iter->value += iter->sizeof_member;
	return iter->value;
};

