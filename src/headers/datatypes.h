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
    u32                      binding;
    u32                      descriptor_count; // idrk
    VkDescriptorBindingFlags flags;
    VkDescriptorType         type;
} DescriptorBindingLayout; // Custom specification of descriptor layout that does not need to be allocated

typedef enum {
    UNIFORM_TYPE_IMAGE,
    UNIFORM_TYPE_FLOAT,
    UNIFORM_TYPE_INT,
    UNIFORM_TYPE_VEC2,
    UNIFORM_TYPE_VEC3,
} UniformType; // all the supported uniform types

typedef struct {
    UniformType uniform_type;
    union {
        VkImage image;
        float number;
        i32 integer;
        float vector2[2];
        float vector3[3];
    } uniform;
    i32 binding;
} Uniforms;

typedef struct {
    VkShaderModule shader; // shader to execute
    float size[2]; // dimensions of the image (let's keep it 2D for simplicity for now)
    Uniforms* uniforms; // all the additional (excluding default ones) uniforms to apply
} ShaderOperarion;

typedef struct {
    char** loaded_modules; // vec of modules names, used to hot reload lua later
    char** loaded_shaders; // vec of shaders names, used to hot reload shader files later
    VkShaderModule* shaders; // vec of shaders used by program
    VkImage* images; // vec of images to be able to free them afterwards
    Program* program; // handle to the fully initialized program to be able to access gpu
} PipelineData;

typedef struct {
    u32 resolution[2];
    float time;

    u8 padding[4]; // std140 requires struct size to be a multiple of 16
} UniformBuffer;

#endif
