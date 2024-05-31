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

void Window::waitEvents() const{
    glfwWaitEvents();
}

void Window::enableCursor(){
    glfwSetInputMode(*window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Window::disableCursor(){
    glfwSetInputMode(*window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

std::pair<int, int> Window::getFramebufferSize() const{
    return {data_.width_, data_.height_};
}

bool Window::shouldClose() const{
    return static_cast<bool>(glfwWindowShouldClose(*window_));
}

void Window::setCallbacks() {
    glfwSetFramebufferSizeCallback(*window_, framebufferResizeInternalCallback);
    glfwSetKeyCallback(*window_, keyDownInternalCallback);
    glfwSetCursorPosCallback(*window_, mouseMovedInternalCallback);
    glfwSetMouseButtonCallback(*window_, mouseButtonInternalCallback);
    glfwSetScrollCallback(*window_, mouseScrollInternalCallback);
    glfwSetCharCallback(*window_, charInputInternalCallback);
    glfwSetWindowFocusCallback(*window_, windowFocusInternalCallback);
}

void Window::framebufferResizeInternalCallback(GLFWwindow* win, int width, int height){
    auto thisWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(win));
    thisWindow->data_.width_ = width;
    thisWindow->data_.height_ = height;

    thisWindow->callbacks.winResized(width, height);
}

void Window::keyDownInternalCallback(GLFWwindow* win, int key, int scancode, int action, int mods) {
    auto thisWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(win));
    if (action == GLFW_PRESS) {
        thisWindow->callbacks.keyDown(key);
    } else if (action == GLFW_RELEASE) {
        thisWindow->callbacks.keyUp(key);
    } else if (action == GLFW_REPEAT) {
        thisWindow->callbacks.keyPressed(key);
    }
}

void Window::mouseMovedInternalCallback(GLFWwindow* win, double xpos, double ypos) {
    auto thisWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(win));
    thisWindow->callbacks.mouseMoved(xpos, ypos);
}

void Window::mouseButtonInternalCallback(GLFWwindow* win, int button, int action, int mods) {
    auto thisWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(win));
    if (action == GLFW_PRESS) {
        thisWindow->callbacks.mouseBtnDown(button);
    } else if (action == GLFW_RELEASE) {
        thisWindow->callbacks.mouseBtnUp(button);
    }
}

void Window::mouseScrollInternalCallback(GLFWwindow* win, double xoffset, double yoffset) {
    auto thisWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(win));
    thisWindow->callbacks.mouseScroll(xoffset, yoffset);
}

void Window::charInputInternalCallback(GLFWwindow* win, unsigned int codepoint) {
    auto thisWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(win));
    thisWindow->callbacks.charInput(codepoint);
}


void Window::windowFocusInternalCallback(GLFWwindow* window, int focused) {
    if (focused) {
        // Скрыть курсор и захватить его
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
        // Показать курсор
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

vk::raii::SurfaceKHR Window::createSurface(const vk::raii::Instance &instance) const{
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(static_cast<VkInstance>(*instance), *window_, nullptr, &surface) != VK_SUCCESS){
        throw std::runtime_error("failed to create window surface!");
    }
    return vk::raii::SurfaceKHR(instance, surface);
}
}
