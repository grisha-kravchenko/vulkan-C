#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan_core.h>

#include "headers/datatypes.h"

void chk_raw(VkResult result, char* file, u32 line) {
	if (result == VK_SUCCESS) return;
	fprintf(stderr, "[FATAL ERROR] Unwrapping unsuccessfull result: %i in [%s:%d]\n", result, file, line);
	exit(1);
}

