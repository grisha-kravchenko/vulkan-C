#ifndef P_DATATYPES
#define P_DATATYPES 1

#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include "thirdparty/vk_mem_alloc.h"

typedef uint32_t u32;
typedef int32_t i32;
typedef uint64_t u64;
typedef int64_t i64;
typedef uint8_t u8;
typedef int8_t i8;

typedef size_t usize;

typedef struct {
    GLFWwindow*      window;
    VkImage*         images;
    VkSemaphore*     submit_semaphores;
    VmaAllocator     allocator;
    u32              queue_family_index;
    VkExtent2D       image_format;
    VkInstance       instance;
    VkPhysicalDevice physical_device;
    VkDevice         device;
    VkQueue          queue;
    VkSwapchainKHR   swapchain;
    VkSurfaceKHR     surface;
    VkSemaphore      image_aviable;
    VkCommandPool    cmd_pool;
    VkCommandBuffer  cmd_buffer;
    VkFence          frame_fence;
#ifdef DEBUG
    VkDebugUtilsMessengerEXT debug_messenger;
#endif
} Program;

typedef struct {
    u32 binding;
    u32 descriptor_count; // idrk
    VkDescriptorBindingFlags flags;
    VkDescriptorType type;
} DescriptorBindingLayout; // Custom specification of descriptor layout that does not need to be allocated

#endif
