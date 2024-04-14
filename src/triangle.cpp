#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <format>
#include <iomanip>
#include <iostream>
#include <set>
#include <stdexcept>
#include <vector>

#include "headers/triangle.h"

// Public
VulkanStarterTriangle::VulkanStarterTriangle(int width, int height) {
    this->width = width;
    this->height = height;

    this->window = nullptr;
    this->instance = nullptr;
}

void VulkanStarterTriangle::run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}


// Private
void VulkanStarterTriangle::initWindow() {
    // Must be the very first call to initialize GLFW
    glfwInit();

    // GLFW was originally designed for OpenGL.
    // This WindowHint tells GLFW to not initialize the window ith OpenGL
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Disable Window Resizing
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(width, height, "Vulkan Triangle", nullptr, nullptr);
}

void VulkanStarterTriangle::initVulkan() { createInstance(); }

void VulkanStarterTriangle::createInstance() {
    // (Optional) Metadata to the driver about this application
    VkApplicationInfo appInfo = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Hello Triangle",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "No Engine",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_API_VERSION_1_0,
    };

    // Fetch all the required Instance Extensions
    uint32_t glfwExtensionCount;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // Display debug information about all available extensions and the ones that will be enabled
    extensionDebugInfo(glfwExtensionCount, glfwExtensions);

    // Define and create a Vulkan instance
    VkInstanceCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &appInfo,
            .enabledExtensionCount = glfwExtensionCount,
            .ppEnabledExtensionNames = glfwExtensions,
    };
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vk Instance!");
    }
}

/**
 * Display all available and enabled extensions on the console
 */
void VulkanStarterTriangle::extensionDebugInfo(uint32_t enabledExtensionCount, const char **enabledExtensions) {
    // Put all the enabled extension names into a set
    std::set<std::string> enabledExtensionNames;
    for (int i = 0; i < enabledExtensionCount; i++) { enabledExtensionNames.insert(enabledExtensions[i]); }

    // Fetch all available availableExtensions
    uint32_t availableExtensionCount;
    vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());

    // Print statements to structure the output as a table
    std::string divider = "-----------------------------------------------------------------";
    std::cout << "Available and Enabled Extensions" << std::endl;
    std::cout << divider << std::endl;
    std::cout << "| ";
    std::cout << std::left << std::setw(10) << "Enabled" << std::setw(0) << "| ";
    std::cout << std::left << std::setw(50) << "Available Extensions" << std::setw(0) << "|";
    std::cout << std::endl;
    std::cout << divider << std::endl;
    for (const auto &extension: availableExtensions) {
        std::cout << "| ";
        std::cout << std::left << std::setw(10)
                  << (enabledExtensionNames.contains(extension.extensionName) ? "Yes" : "") << std::setw(0) << "| ";
        std::cout << std::left << std::setw(50) << extension.extensionName << std::setw(0) << "|";
        std::cout << std::endl;
    }
    std::cout << divider << std::endl;
}

void VulkanStarterTriangle::mainLoop() {
    // Keep the window open until it is closed (or an error occurs)
    while (!glfwWindowShouldClose(window)) { glfwPollEvents(); }
}

void VulkanStarterTriangle::cleanup() {
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}
