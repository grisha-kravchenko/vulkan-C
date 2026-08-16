#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan_core.h>

#include "headers/datatypes.h"
#include "headers/misc.h"
#include "headers/thirdparty/vk_mem_alloc.h"

void chk_raw(VkResult result, char* file, u32 line) {
    if (result == VK_SUCCESS) return;
    fprintf(stderr, "[FATAL ERROR] Unwrapping unsuccessfull result: %i in [%s:%d]\n", result, file, line);
    abort();
}

void on_close(Program *program) {
    vkDestroySwapchainKHR(program->device, program->swapchain, NULL);
    vkDestroySurfaceKHR(program->instance, program->surface, NULL);
    vkFreeCommandBuffers(program->device, program->cmd_pool, 1, &program->cmd_buffer);
    vkDestroySemaphore(program->device, program->image_aviable, NULL);

    // Destroy all the previous semaphores
    if (program->submit_semaphores != NULL) for (size_t i = 0; i < vec_len(program->submit_semaphores); ++i) {
        vkDestroySemaphore(program->device, program->submit_semaphores[i], NULL);
    }

    vkDestroyFence(program->device, program->frame_fence, NULL);
    vkDestroyCommandPool(program->device, program->cmd_pool, NULL);

    vmaDestroyAllocator(program->allocator);
    vkDestroyDevice(program->device, NULL);

    glfwDestroyWindow(program->window);
    glfwTerminate();

#ifdef DEBUG
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT =
        (PFN_vkDestroyDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(program->instance, "vkDestroyDebugUtilsMessengerEXT");
    vkDestroyDebugUtilsMessengerEXT(program->instance, program->debug_messenger, NULL);
#endif

    vkDestroyInstance(program->instance, NULL);
}

VkSurfaceCapabilitiesKHR get_capabilities(Program* program) {
    VkSurfaceCapabilitiesKHR caps = {0};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(program->physical_device, program->surface, &caps);
    return caps;
}

