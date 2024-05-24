#include "window.hpp"

namespace rendr
{
//Vulkan instance extensions required by GLFW
std::vector<const char*> Window::getRequiredExtensions() const{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    return extensions;
}

void Window::pollEvents() const{
    glfwPollEvents();
}  

std::pair<int, int> Window::getFramebufferSize() const{
    std::pair<int, int> size;
    glfwGetFramebufferSize(*window_, &size.first, &size.second);
    return size;
}

bool Window::shouldClose() const{
    return static_cast<bool>(glfwWindowShouldClose(*window_));
}

void Window::setCallbacks(){
    glfwSetFramebufferSizeCallback(*window_, framebufferResizeInternalCallback);
}

void Window::framebufferResizeInternalCallback(GLFWwindow* win, int width, int height){
    auto thisWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(win));
    thisWindow->callbacks.winResized(width, height);
}

vk::raii::SurfaceKHR Window::createSurface(const vk::raii::Instance &instance) const{
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(static_cast<VkInstance>(*instance), *window_, nullptr, &surface) != VK_SUCCESS){
        throw std::runtime_error("failed to create window surface!");
    }
    return vk::raii::SurfaceKHR(instance, surface);
}
}
