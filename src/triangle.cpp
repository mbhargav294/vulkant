#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <algorithm>  // For std::clamp in chooseSwapExtent
#include <cstdint>    // For uint32_t
#include <cstring>
#include <limits>  // For std::numeric_limits in chooseSwapExtent
#include <stdexcept>
#include <vector>

#include "headers/triangle.h"

// Public
VulkanStarterTriangle::VulkanStarterTriangle(int width, int height) {
    this->width = width;
    this->height = height;
    // TODO: Parametrize these values to the constructor
    this->validationLayers = {"VK_LAYER_KHRONOS_validation"};
    this->deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    this->window = VK_NULL_HANDLE;
    this->instance = VK_NULL_HANDLE;

    this->debugMessenger = VK_NULL_HANDLE;
    this->physicalDevice = VK_NULL_HANDLE;

    this->device = VK_NULL_HANDLE;
    this->surface = VK_NULL_HANDLE;

    this->graphicsQueue = VK_NULL_HANDLE;
    this->presentQueue = VK_NULL_HANDLE;

    this->swapChain = VK_NULL_HANDLE;
    this->swapChainImageFormat = VK_FORMAT_UNDEFINED;
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

    window = glfwCreateWindow(width, height, "Vulkan Triangle", VK_NULL_HANDLE, VK_NULL_HANDLE);
}

void VulkanStarterTriangle::initVulkan() {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
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
    createInfo.pNext = VK_NULL_HANDLE;
#else
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();

    populateDebugMessengerCreateInfo(debugCreateInfo);
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debugCreateInfo;
#endif
    if (vkCreateInstance(&createInfo, VK_NULL_HANDLE, &instance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vk Instance!");
    }
}

void VulkanStarterTriangle::mainLoop() {
    // Keep the window open until it is closed (or an error occurs)
    while (!glfwWindowShouldClose(window)) { glfwPollEvents(); }
}

void VulkanStarterTriangle::cleanup() {
#ifndef NDEBUG
    DestroyDebugUtilsMessengerEXT(instance, debugMessenger, VK_NULL_HANDLE);
#endif
    vkDestroySwapchainKHR(device, swapChain, VK_NULL_HANDLE);
    vkDestroyDevice(device, VK_NULL_HANDLE);
    vkDestroySurfaceKHR(instance, surface, VK_NULL_HANDLE);
    vkDestroyInstance(instance, VK_NULL_HANDLE);
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool VulkanStarterTriangle::checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, VK_NULL_HANDLE);
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

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, VK_NULL_HANDLE, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("Failed to set up debug messenger!");
    }
}

void VulkanStarterTriangle::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, VK_NULL_HANDLE);

    if (deviceCount == 0) { throw std::runtime_error("Failed to find GPUs with Vulkan support!"); }

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

    int maxScore = 0;
    VkPhysicalDevice selectedDevice = VK_NULL_HANDLE;
    for (const auto &physicaDevice: physicalDevices) {
        int score = deviceScore(physicaDevice);
        if (0 <= score) {
            maxScore = score;
            selectedDevice = physicaDevice;
        }
    }

    if (0 == maxScore || !isDeviceSuitable(selectedDevice)) {
        throw std::runtime_error("Failed to find suitable GPU!");
    }
    physicalDevice = selectedDevice;
}

void VulkanStarterTriangle::createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presetFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily: uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = queueFamily,
                .queueCount = 1,
                .pQueuePriorities = &queuePriority,
        };

        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
            .ppEnabledExtensionNames = deviceExtensions.data(),
            .pEnabledFeatures = &deviceFeatures,
    };

#ifdef NDEBUG
    createInfo.enabledLayerCount = 0;
#else
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
#endif

    if (vkCreateDevice(physicalDevice, &createInfo, VK_NULL_HANDLE, &device) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Logical Device!");
    }

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presetFamily.value(), 0, &presentQueue);
}

void VulkanStarterTriangle::createSurface() {
    VkWin32SurfaceCreateInfoKHR createInfo{
            .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .hinstance = GetModuleHandle(VK_NULL_HANDLE),
            .hwnd = glfwGetWin32Window(window),
    };

    if (vkCreateWin32SurfaceKHR(instance, &createInfo, VK_NULL_HANDLE, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }
}

VulkanStarterTriangle::QueueFamilyIndices VulkanStarterTriangle::findQueueFamilies(VkPhysicalDevice pDevice) {
    QueueFamilyIndices indices{};

    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &queueFamilyCount, VK_NULL_HANDLE);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &queueFamilyCount, queueFamilies.data());

    for (int i = 0; i < queueFamilyCount; i++) {
        // Right most bit in the Queue Flags will be set if the queue supports Graphics
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) { indices.graphicsFamily = i; }

        VkBool32 presentSupport;
        vkGetPhysicalDeviceSurfaceSupportKHR(pDevice, i, surface, &presentSupport);
        if (presentSupport) { indices.presetFamily = i; }

        if (indices.isComplete()) { break; }
    }

    return indices;
}

bool VulkanStarterTriangle::isDeviceSuitable(VkPhysicalDevice pDevice) {
    QueueFamilyIndices indices = findQueueFamilies(pDevice);

    bool extensionsSupported = checkDeviceExtensionSupport(pDevice);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(pDevice);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

bool VulkanStarterTriangle::checkDeviceExtensionSupport(VkPhysicalDevice pDevice) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(pDevice, VK_NULL_HANDLE, &extensionCount, VK_NULL_HANDLE);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(pDevice, VK_NULL_HANDLE, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto &extension: availableExtensions) { requiredExtensions.erase(extension.extensionName); }

    return requiredExtensions.empty();
}

void VulkanStarterTriangle::createSwapChain() {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = surface,
            .minImageCount = imageCount,
            .imageFormat = surfaceFormat.format,
            .imageColorSpace = surfaceFormat.colorSpace,
            .imageExtent = extent,
            .imageArrayLayers = 1,                              // Must always be 1, except for Stereoscopic 3D
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,  // Directly render to screen without any postprocessing
    };

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = {
            indices.graphicsFamily.value(),
            indices.presetFamily.value(),
    };

    if (indices.graphicsFamily != indices.presetFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;             // Optional
        createInfo.pQueueFamilyIndices = VK_NULL_HANDLE;  // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;  // Ignore alpha, used for window transparency
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;              // Will not compute the pixels obscured by other windows
    createInfo.oldSwapchain = VK_NULL_HANDLE;  // Resizing windows not allowed

    if (vkCreateSwapchainKHR(device, &createInfo, VK_NULL_HANDLE, &swapChain) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, VK_NULL_HANDLE);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

VulkanStarterTriangle::SwapChainSupportDetails VulkanStarterTriangle::querySwapChainSupport(VkPhysicalDevice pDevice) {
    VulkanStarterTriangle::SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pDevice, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, surface, &formatCount, VK_NULL_HANDLE);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, surface, &presentModeCount, VK_NULL_HANDLE);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR
VulkanStarterTriangle::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    for (const auto &availableFormat: availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR
VulkanStarterTriangle::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
    // Similar to VSync
    //    return VK_PRESENT_MODE_FIFO_KHR;

    // Triple Buffering
    //    return VK_PRESENT_MODE_MAILBOX_KHR;

    // Might result in tearing
    //    return VK_PRESENT_MODE_IMMEDIATE_KHR;

    for (const auto &presentMode: availablePresentModes) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) { return presentMode; }
    }

    // This should be always present
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanStarterTriangle::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);

        VkExtent2D actualExtent = {static_cast<uint32_t>(w), static_cast<uint32_t>(h)};

        actualExtent.width =
                std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height =
                std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}
