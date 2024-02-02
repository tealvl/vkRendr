#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class HelloTriangleApplication {
public:
    void run();
private:
    GLFWwindow* window;

    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();
};