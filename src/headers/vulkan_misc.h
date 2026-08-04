#ifndef P_VULKAN_MISC
#define P_VULKAN_MISC 1

#include <vulkan/vulkan_core.h>
#include "datatypes.h"

void on_close(Program *program);
void chk_raw(VkResult result, char* file, u32 line);
VkSurfaceCapabilitiesKHR get_capabilities(Program* program);

#define chk(result) chk_raw(result, __FILE__, __LINE__)
#define IMAGE_FORMAT VK_FORMAT_R8G8B8A8_UNORM
#define COLOR_SPACE VK_COLOR_SPACE_SRGB_NONLINEAR_KHR

#endif
