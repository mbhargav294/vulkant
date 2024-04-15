#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstring>
#include <format>
#include <stdexcept>
#include <vector>

#include "headers/triangle.h"

// Public
VulkanStarterTriangle::VulkanStarterTriangle(int width, int height) {
    this->width = width;
    this->height = height;

    this->window = nullptr;
    this->instance = nullptr;
    this->validationLayers = {"VK_LAYER_KHRONOS_validation"};

    this->physicalDevice = VK_NULL_HANDLE;
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

void VulkanStarterTriangle::initVulkan() {
    createInstance();
    setupDebugMessenger();
    pickPhysicalDevice();
}

void VulkanStarterTriangle::createInstance() {
#ifndef NDEBUG
    if (!checkValidationLayerSupport()) { throw std::runtime_error("validation layers requested, but not available!"); }
#endif

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
    std::vector<const char *> extensions = getRequiredExtensions();

    // Display debug information about all available extensions and the ones that will be enabled
    extensionDebugInfo(extensions);

    // Define and create a Vulkan instance
    VkInstanceCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &appInfo,
            .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data(),
    };
    // Add validation layers and debug logs in debug mode
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
#ifdef NDEBUG
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
#else
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();

    populateDebugMessengerCreateInfo(debugCreateInfo);
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debugCreateInfo;
#endif
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vk Instance!");
    }
}

void VulkanStarterTriangle::mainLoop() {
    // Keep the window open until it is closed (or an error occurs)
    while (!glfwWindowShouldClose(window)) { glfwPollEvents(); }
}

void VulkanStarterTriangle::cleanup() {
#ifndef NDEBUG
    DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
#endif
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool VulkanStarterTriangle::checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layer: validationLayers) {
        for (const VkLayerProperties &availableLayer: availableLayers) {
            if (strcmp(layer, availableLayer.layerName) == 0) { return true; }
        }
    }

    return false;
}

void VulkanStarterTriangle::setupDebugMessenger() {
#ifdef NDEBUG
    return;
#endif

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("Failed to set up debug messenger!");
    }
}

void VulkanStarterTriangle::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) { throw std::runtime_error("Failed to find GPUs with Vulkan support!"); }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    int maxScore = 0;
    VkPhysicalDevice selectedDevice = VK_NULL_HANDLE;
    for (const auto &device: devices) {
        int score = deviceScore(device);
        if (0 <= score) {
            maxScore = score;
            selectedDevice = device;
        }
    }

    if (0 == maxScore || !isDeviceSuitable(selectedDevice)) {
        throw std::runtime_error("Failed to find suitable GPU!");
    }
    physicalDevice = selectedDevice;
}
