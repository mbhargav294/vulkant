#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iomanip>
#include <iostream>
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

    void initWindow();
    void createInstance();
    void initVulkan();
    void mainLoop();
    void cleanup();
    bool checkValidationLayerSupport();
    void setupDebugMessenger();


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
        vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());

        // Print statements to structure the output as a table
        std::string divider = "-----------------------------------------------------------------";
        std::cout << divider << std::endl;
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
                .pUserData = nullptr,
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
        if (nullptr != func) {
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
        if (nullptr != func) { func(instance, debugMessenger, pAllocator); }
    }
};

#endif  //STARTER_TRIANGLE_H
