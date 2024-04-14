#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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

    void initWindow();
    void createInstance();
    void initVulkan();
    void mainLoop();
    void cleanup();

    static void extensionDebugInfo(uint32_t enabledExtensionCount, const char **enabledExtensions);
};

#endif  //STARTER_TRIANGLE_H
