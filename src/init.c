#include <assert.h>
#include <stdio.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>

#include "headers/datatypes.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback (
	VkDebugUtilsMessageSeverityFlagBitsEXT severity,
	VkDebugUtilsMessageTypeFlagsEXT type,
	const VkDebugUtilsMessengerCallbackDataEXT* data,
	void* user_data
) {
	if (severity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		return VK_FALSE;
	printf("Validation layer: %u, msg: %s", type, data->pMessage);
	return VK_FALSE;
}

void initialise(Program* app) {
	VkInstance instance = {0};
	app->instance = instance;

	u32 property_count = 0;
	vkEnumerateInstanceLayerProperties(&property_count, NULL);

	VkLayerProperties layer_propeties[property_count];
	vkEnumerateInstanceLayerProperties(&property_count, layer_propeties);

	char* valdation_layer_name = "VK_LAYER_KHRONOS_validation";
	char* enabled_layers[] = { valdation_layer_name };
	u32 enabled_layers_len = sizeof(enabled_layers) / sizeof(char*);

	u32 glfw_extension_count = 0;
	const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

	VkApplicationInfo app_info = {
		VK_STRUCTURE_TYPE_APPLICATION_INFO, NULL,
		"Vulkan & C! :D", 1, "Vulkan-C engine", 1,
		VK_API_VERSION_1_4
	};

	VkInstanceCreateInfo create_info = {
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, NULL,
		0, &app_info,
		enabled_layers_len, (char const **)enabled_layers,
		glfw_extension_count, glfw_extensions,
	};
	if (!vkCreateInstance(&create_info, NULL, &app->instance))
		assert(!"[FATAL ERROR] Failed to create vulkan instance");
}
