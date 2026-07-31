#ifndef P_VULKAN_MISC
#define P_VULKAN_MISC 1

#include <vulkan/vulkan_core.h>
#include "datatypes.h"

void chk_raw(VkResult result, char* file, u32 line);
#define chk(result) chk_raw(result, __FILE__, __LINE__)

#endif
