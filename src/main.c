#define VK_USE_PLATFORM_WAYLAND_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WAYLAND
#include <GLFW/glfw3native.h>

#include <stdio.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "headers/datatypes.h"
#include "headers/init.h"
#include "headers/vulkan_misc.h"
#include "headers/window.h"

int main() {
    Program program = {0};
    initialise(&program);
    init_glfw(&program);

    printf("[INFO]: Successfully initialised the program.\n");
    window_code(&program);
    printf("[INFO]: Exiting the application.\n");

    on_close(&program);
}
