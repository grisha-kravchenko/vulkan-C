#ifndef P_DATATYPES
#define P_DATATYPES 1

#include <vulkan/vulkan_core.h>
#include <stdlib.h>

typedef uint32_t u32;
typedef int32_t i32;
typedef uint64_t u64;
typedef int64_t i64;
typedef uint8_t u8;
typedef int8_t i8;

typedef size_t usize;

typedef struct {
	usize count;
	usize capacity;
	void* values;
	usize sizeof_member;
} Vec;

typedef struct {
	usize count;
	void* value;
	usize sizeof_member;
} Iter;

typedef struct {
	VkInstance       instance;
	VkPhysicalDevice physical_device;
	VkDevice         device;
	VkQueue          queue;
#ifdef DEBUG
	VkDebugUtilsMessengerEXT debug_messenger;
#endif
} Program;

#endif
