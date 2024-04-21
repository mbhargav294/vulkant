#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <format>
#include <iomanip>
#include <iostream>
#include <optional>
#include <set>
#include <vector>

#ifndef STARTER_TRIANGLE_H
#define STARTER_TRIANGLE_H


class VulkanStarterTriangle {
public:
    VulkanStarterTriangle(int width, int height);
    void run();

private:
    int width;
    int height;
    GLFWwindow *window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    std::vector<const char *> validationLayers;
    std::vector<const char *> deviceExtensions;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkSurfaceKHR surface;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent{};
    std::vector<VkImageView> swapChainImageViews;

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presetFamily;

        [[nodiscard]] bool isComplete() const { return graphicsFamily.has_value() && presetFamily.has_value(); }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    void initWindow();
    void createInstance();
    void initVulkan();
    void mainLoop();
    void cleanup();
    bool checkValidationLayerSupport();
    void setupDebugMessenger();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSurface();
    void createSwapChain();
    void createImageViews();
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice pDevice);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice pDevice);
    bool isDeviceSuitable(VkPhysicalDevice pDevice);
    bool checkDeviceExtensionSupport(VkPhysicalDevice pDevice);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentMode);

    static constexpr std::string_view divider = "|---------------------------------------------------------------|";

    static std::vector<const char *> getRequiredExtensions() {
        {
            uint32_t glfwExtensionsCount;
            const char **glfwExtensions;
            glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

            std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);

#ifndef NDEBUG
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

            return extensions;
        }
    }

    /**
     * Display all available and enabled extensions on the console
     */
    static void extensionDebugInfo(std::vector<const char *> enabledExtensions) {
        // Put all the enabled extension names into a set
        std::set<std::string> enabledExtensionNames(enabledExtensions.begin(), enabledExtensions.end());

        // Fetch all available availableExtensions
        uint32_t availableExtensionCount;
        vkEnumerateInstanceExtensionProperties(VK_NULL_HANDLE, &availableExtensionCount, VK_NULL_HANDLE);
        std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
        vkEnumerateInstanceExtensionProperties(VK_NULL_HANDLE, &availableExtensionCount, availableExtensions.data());

        // Print statements to structure the output as a table
        std::cout << std::endl << "Available and Enabled Extensions" << std::endl;
        std::cout << divider << std::endl;
        printTableLine("Enabled", "Available Extensions", 10, 50);
        std::cout << divider << std::endl;
        for (const auto &extension: availableExtensions) {
            printTableLine((enabledExtensionNames.contains(extension.extensionName) ? "Yes" : ""),
                           extension.extensionName, 10, 50);
        }
        std::cout << divider << std::endl;
    }

    /**
     * Callback function used in setupDebugMessenger to log the messages thrown by Vulkan
     *
     * @param messageSeverity
     * @param messageType
     * @param pCallbackData
     * @param pUserData
     * @return
     */
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                        void *pUserData) {
        // Show only WARN or above logs
        if (messageSeverity <= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) { return VK_FALSE; }

        std::string logType;
        switch (messageSeverity) {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                logType = "[VERBOSE]";
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                logType = "[INFO]";
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                logType = "[WARN]";
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                logType = "[ERROR]";
                break;
            default:
                logType = "[NONE]";
        }
        std::string prefix;
        switch (messageType) {
            case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
                prefix = "General";
                break;
            case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
                prefix = "Validation";
                break;
            case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
                prefix = "Performance";
                break;
            default:
                prefix = "[NONE]";
        }

        std::cout << logType << " " << prefix << ":" << std::endl << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
        createInfo = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                .messageSeverity =
                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                .pfnUserCallback = debugCallback,
                .pUserData = VK_NULL_HANDLE,
        };
    }

    /**
     * vkCreateDebugUtilsMessengerEXT is an extension function is not automatically loaded.
     * This is a proxy function to load vkCreateDebugUtilsMessengerEXT using the method name and call it.
     *
     * @param instance
     * @param pCreateInfo
     * @param pAllocator
     * @param pDebugMessenger
     * @return
     */
    static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                                 const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator,
                                                 VkDebugUtilsMessengerEXT *pDebugMessenger) {
        auto func =
                (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (VK_NULL_HANDLE != func) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    /**
     * vkDestroyDebugUtilsMessengerEXT is an extension function is not automatically loaded.
     * This is a proxy function to load vkDestroyDebugUtilsMessengerEXT using the method name and call it.
     *
     * @param instance
     * @param debugMessenger
     * @param pAllocator
     */
    static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                              const VkAllocationCallbacks *pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,
                                                                                "vkDestroyDebugUtilsMessengerEXT");
        if (VK_NULL_HANDLE != func) { func(instance, debugMessenger, pAllocator); }
    }

    /**
     * Checks if the device is the Discrete GPU and if it supports Geometry Shader
     *
     * @param device
     * @return
     */
    static int deviceScore(VkPhysicalDevice device) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        std::cout << std::endl << "Listing available GPUs" << std::endl;
        std::cout << divider << std::endl;
        printTableLine("Name", deviceProperties.deviceName, 30, 30);
        std::cout << divider << std::endl;
        printTableLine("ID", std::format("{}", deviceProperties.deviceID), 30, 30);
        printTableLine("Vendor ID", std::format("{}", deviceProperties.vendorID), 30, 30);
        printTableLine("Max 2D image dimensions", std::format("{}", deviceProperties.limits.maxImageDimension2D), 30,
                       30);
        printTableLine("API Version", std::format("{}", deviceProperties.apiVersion), 30, 30);
        printTableLine("Driver Version", std::format("{}", deviceProperties.driverVersion), 30, 30);
        printTableLine("Geometry Shader", std::format("{}", deviceFeatures.geometryShader), 30, 30);
        std::cout << divider << std::endl;

        int score = 0;

        // Add maximum score for discrete GPUs
        if (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU == deviceProperties.deviceType) { score += 1000; }

        // Add maximum supported texture size to overall score
        score += static_cast<int>(deviceProperties.limits.maxImageDimension2D);

        // If GPU does not support Geometry Shader
        if (!deviceFeatures.geometryShader) { score = 0; }

        printTableLine("Score", std::format("{}", score), 30, 30);
        std::cout << divider << std::endl;

        return score;
    }

    /**
     * Print a single row to console in a tabular format
     *
     * @param col1
     * @param col2
     * @param col1Width
     * @param col2Width
     */
    static void printTableLine(const std::string &col1, const std::string &col2, int col1Width, int col2Width) {
        std::cout << std::left << "| " << std::setw(col1Width) << col1 << "| " << std::setw(col2Width) << col2
                  << std::setw(0) << "|" << std::endl;
    }
};

#endif  //STARTER_TRIANGLE_H
