#include <assert.h>
#include <stdio.h>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_wayland.h>

#include "headers/datatypes.h"
#include "headers/misc.h"
#include "headers/vulkan_misc.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback (
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* data,
    void* user_data
) {
    Program* program = user_data;
    if (severity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        return VK_FALSE;
    char* severity_string =
        severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT ? "WARN" :
        severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ? "ERROR" :
        "UNKNOWN";
    fprintf(stderr, "[Validation Layer | %s]: %s\n\n", severity_string, data->pMessage);
    return VK_FALSE;
}

void setup_debugger(Program* program) {
    VkDebugUtilsMessageSeverityFlagBitsEXT severity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    VkDebugUtilsMessageTypeFlagBitsEXT message_type =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT create_info = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageType = message_type, .messageSeverity = severity,
        .pfnUserCallback = debug_callback,
        .pUserData = &program
    };

    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
        (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(program->instance, "vkCreateDebugUtilsMessengerEXT");
    assert(vkCreateDebugUtilsMessengerEXT != NULL &&
        "[FATAL ERROR] Couldn't get debug utils messenger creator");

    VkDebugUtilsMessengerEXT messenger = {0};
    chk(vkCreateDebugUtilsMessengerEXT(program->instance, &create_info, NULL, &messenger));
#ifdef DEBUG
    program->debug_messenger = messenger;
#endif
}

void initialise(Program* program) {
    VkInstance instance = {0};
    program->instance = instance;

    u32 property_count = 0;
    vkEnumerateInstanceLayerProperties(&property_count, NULL);

#ifdef DEBUG
    VkLayerProperties layer_propeties[property_count];
    vkEnumerateInstanceLayerProperties(&property_count, layer_propeties);

    char* valdation_layer_name = "VK_LAYER_KHRONOS_validation";
    char* enabled_layers[] = { valdation_layer_name };
    u32 enabled_layers_len = sizeof(enabled_layers) / sizeof(char*);
#endif

    u32 glfw_extension_count = 0;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    const char** instance_extensions = NULL;
    vec_concat_array(instance_extensions, glfw_extensions, glfw_extension_count);
    vec_push(instance_extensions, VK_KHR_SURFACE_EXTENSION_NAME);
    vec_push(instance_extensions, VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);

#ifdef DEBUG
    vec_push(instance_extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    VkApplicationInfo app_info = {
        VK_STRUCTURE_TYPE_APPLICATION_INFO, NULL,
        "Vulkan & C! :D", 1, "Vulkan-C engine", 1,
        VK_API_VERSION_1_4
    };

    VkInstanceCreateInfo create_info = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, NULL,
        0, &app_info,
#ifdef DEBUG
        enabled_layers_len, (char const **)enabled_layers,
#else
        0, NULL,
#endif
        vec_len(instance_extensions), instance_extensions,
    };
    chk(vkCreateInstance(&create_info, NULL, &program->instance));
    vec_free(instance_extensions);

#ifdef DEBUG
    setup_debugger(program);
#endif

    u32 device_count = 0;
    chk(vkEnumeratePhysicalDevices(program->instance, &device_count, NULL));
    VkPhysicalDevice devices[device_count];
    chk(vkEnumeratePhysicalDevices(program->instance, &device_count, devices));

    u32 selected_device = 0;
    i32 type = 999;
    for (u32 i = 0; i < device_count; ++i) {
        VkPhysicalDeviceProperties propeties = {0};
        vkGetPhysicalDeviceProperties(devices[i], &propeties);

        i32 device_type = 999;
        switch (propeties.deviceType) {
            case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                device_type = 4; break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                device_type = 3; break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                device_type = 2; break;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                device_type = 1; break;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                device_type = 0; break;
            default:
                assert(!"Unreachable");
        }

        if (device_type < type) {
            type = device_type;
            selected_device = i;
        }
    }

    program->physical_device = devices[selected_device];

    VkPhysicalDeviceProperties propeties = {0};
    vkGetPhysicalDeviceProperties(program->physical_device, &propeties);

    printf("[INFO]: Selected device: [%s] %s \n",
        propeties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? "Discrete GPU" :
        propeties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ? "Integrated GPU" :
        propeties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU ? "Virtual GPU" :
        propeties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU ? "CPU" : "UNKNOWN",
         propeties.deviceName
    );

    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(program->physical_device, &queue_family_count, NULL);
    VkQueueFamilyProperties queue_family_properties[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(program->physical_device, &queue_family_count, queue_family_properties);
    u32 queue_family = 0;

    for (u32 i = 0; i < queue_family_count; ++i) {
        if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queue_family = i;
            break;
        }
    }

    program->queue_family_index = queue_family;

    float queue_priorities[] = { 0.5f };
    VkDeviceQueueCreateInfo queue_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .flags = 0,
        .queueFamilyIndex = queue_family,
        .queueCount = 1,
        .pQueuePriorities = queue_priorities,
    };

    VkPhysicalDeviceVulkan12Features enabled_12_features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .descriptorIndexing = VK_TRUE,
        .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
        .descriptorBindingVariableDescriptorCount = VK_TRUE,
        .runtimeDescriptorArray = VK_TRUE,
        .bufferDeviceAddress = VK_TRUE,
    };
    VkPhysicalDeviceVulkan13Features enabled_13_features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &enabled_12_features,
        .synchronization2 = VK_TRUE,
        .dynamicRendering = VK_TRUE,
    };
    VkPhysicalDeviceVulkan14Features enabled_14_features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
        .pNext = &enabled_13_features,
    };
    VkPhysicalDeviceFeatures enabled_10_features = {
        .samplerAnisotropy = VK_TRUE,
    };

    const char* device_extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, };

    VkDeviceCreateInfo device_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = &queue_create_info,
        .queueCreateInfoCount = 1,
        .pNext = &enabled_14_features,
        .ppEnabledExtensionNames = device_extensions,
        .enabledExtensionCount = 1,
        .pEnabledFeatures = &enabled_10_features,
    };

    chk(vkCreateDevice(program->physical_device, &device_create_info, NULL, &program->device));
    vkGetDeviceQueue(program->device, queue_family, 0, &program->queue);
    assert(program->queue);

    VkSemaphoreCreateInfo sem_create_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    vkCreateSemaphore(program->device, &sem_create_info, NULL, &program->image_aviable);
    // vkCreateSemaphore(program->device, &sem_create_info, NULL, &program->render_finished);

    VkCommandPoolCreateInfo cmd_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queue_family,
    };

    chk(vkCreateCommandPool(program->device, &cmd_pool_create_info, NULL, &program->cmd_pool));

    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = program->cmd_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    chk(vkAllocateCommandBuffers(program->device, &alloc_info, &program->cmd_buffer));

    VkFenceCreateInfo fence_info = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, NULL, VK_FENCE_CREATE_SIGNALED_BIT };

    vkCreateFence(program->device, &fence_info, NULL, &program->frame_fence);
}


