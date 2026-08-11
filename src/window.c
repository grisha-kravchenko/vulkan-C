#define VK_USE_PLATFORM_WAYLAND_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WAYLAND
#include <GLFW/glfw3native.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <math.h>

#include "headers/vulkan_misc.h"
#include "headers/datatypes.h"
#include "headers/window.h"
#include "headers/misc.h"

static void error_callback(int error, const char* description) {
    fprintf(stderr, "[GLFW Error]: %s\n\n", description);
}

void init_glfw(Program* program) {
    try(glfwInit(), "Couldn't initialise glfw");
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // disable opengl
    // glfwWindowHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);

#ifdef DEBUG // setup debugger
    glfwSetErrorCallback(error_callback);
#endif

    try_c(glfwVulkanSupported(), "Vulkan is not supported by glfw", glfwTerminate);

    program->window = glfwCreateWindow(800, 600, "Vulkan engine", NULL, NULL);
    try_c(program->window, "Couldn't create glfw window", glfwTerminate);

    chk(glfwCreateWindowSurface(program->instance, program->window, NULL, &program->surface));
    create_swapchain(program);
}

void transition_image(Program* program, VkImage src_image, VkImageLayout old_layout, VkImageLayout new_layout) {
    VkImageSubresourceRange range = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .baseArrayLayer = 0,
        .layerCount = 1,
        .levelCount = 1
    };

    VkImageMemoryBarrier2 image_barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = program->queue_family_index,
        .dstQueueFamilyIndex = program->queue_family_index,
        .image = src_image,
        .subresourceRange = range,
    };

    VkDependencyInfo dependency_info = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &image_barrier,
    };

    vkCmdPipelineBarrier2(program->cmd_buffer, &dependency_info);
}

void window_code(Program* program) {
    PFN_vkCmdClearColorImage vkCmdClearColorImage =
        (PFN_vkCmdClearColorImage)
        vkGetInstanceProcAddr(program->instance, "vkCmdClearColorImage");
    assert(vkCmdClearColorImage != NULL &&
        "[FATAL ERROR] Couldn't vkCmdClearColorImage method");

    while (!glfwWindowShouldClose(program->window)) {
        double time = glfwGetTime();
        try(time, "Couldn't get glfw time.");

        vkWaitForFences(program->device, 1, &program->frame_fence, VK_TRUE, UINT64_MAX);
        vkResetFences(program->device, 1, &program->frame_fence);

        u32 image_index = UINT32_MAX;
        VkResult res = vkAcquireNextImageKHR(
            program->device,
            program->swapchain,
            UINT32_MAX,
            program->image_aviable,
            VK_NULL_HANDLE,
            &image_index
        );

        if (res == VK_ERROR_OUT_OF_DATE_KHR) {
            create_swapchain(program);
            continue;
        } else chk(res);
        try(image_index < vec_len(program->images), "Failed to get image from swapchain");

        vkResetCommandBuffer(program->cmd_buffer, 0);
        VkCommandBufferBeginInfo begin_info = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        vkBeginCommandBuffer(program->cmd_buffer, &begin_info);

        VkImageSubresourceRange range = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1,
            .levelCount = 1
        };

        // Actual drawing here
        VkClearColorValue color = {(float)sin(time) / 2.0 + .5, .5, 1, 1};

        transition_image(program, program->images[image_index], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
        vkCmdClearColorImage(
            program->cmd_buffer,
            program->images[image_index],
            VK_IMAGE_LAYOUT_GENERAL,
            &color, 1, &range
        );

        transition_image(program, program->images[image_index], VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        vkEndCommandBuffer(program->cmd_buffer);

        VkPipelineStageFlags wait_stages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        };

        VkSubmitInfo submit_info = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &program->image_aviable,
            .pWaitDstStageMask = wait_stages,
            .commandBufferCount = 1,
            .pCommandBuffers = &program->cmd_buffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = program->submit_semaphores + image_index,
        };

        chk(vkQueueSubmit(program->queue, 1, &submit_info, program->frame_fence));

        VkPresentInfoKHR present_info = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = program->submit_semaphores + image_index,
            .swapchainCount = 1,
            .pSwapchains = &program->swapchain,
            .pImageIndices = &image_index,
        };

        VkResult res2 = vkQueuePresentKHR(program->queue, &present_info);
        if (res == VK_ERROR_OUT_OF_DATE_KHR) {
            create_swapchain(program);
            continue;
        } else chk(res);

        glfwPollEvents();
    }

    vkWaitForFences(program->device, 1, &program->frame_fence, VK_TRUE, UINT64_MAX);
}

void create_swapchain(Program* program) {
    VkSurfaceCapabilitiesKHR capabilities = get_capabilities(program);
    VkExtent2D image_size = capabilities.currentExtent;
    if (image_size.width == 0xFFFFFFFF) {
        int width = 0; int height = 0;
        glfwGetWindowSize(program->window, &width, &height);

        image_size.width = (u32)width;
        image_size.height = (u32)height;
    }

    VkSwapchainCreateInfoKHR create_info = {
        .sType          = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface        = program->surface, .minImageCount    = capabilities.minImageCount,
        .imageFormat    = IMAGE_FORMAT,     .imageColorSpace  = COLOR_SPACE,
        .imageExtent    = image_size,       .imageArrayLayers = 1,
        .imageUsage     = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
        .preTransform   = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode    = VK_PRESENT_MODE_FIFO_KHR,
    };

    chk(vkCreateSwapchainKHR(program->device, &create_info, NULL, &program->swapchain));

    u32 image_count = 0;
    chk(vkGetSwapchainImagesKHR(program->device, program->swapchain, &image_count, NULL));
    VkImage* images = NULL;
    vec_sized(images, image_count);
    chk(vkGetSwapchainImagesKHR(program->device, program->swapchain, &image_count, images));
    vec_len(images) = image_count;

    vec_free(program->images);
    program->images = images;

    VkSemaphore* semaphores = NULL;
    vec_sized(semaphores, image_count);

    VkSemaphoreCreateInfo sem_create_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    // Allocate all the new semaphores
    for (size_t i = 0; i < image_count; ++i) {
        vkCreateSemaphore(program->device, &sem_create_info, NULL, semaphores + i);
    }
    vec_len(semaphores) = image_count;

    // Destroy all the previous semaphores
    if (program->submit_semaphores != NULL)
    for (size_t i = 0; i < vec_len(program->submit_semaphores); ++i) {
        vkDestroySemaphore(program->device, program->submit_semaphores[i], NULL);
    }

    vec_free(program->submit_semaphores);
    program->submit_semaphores = semaphores;

    // TODO: Attach depth buffer
}
