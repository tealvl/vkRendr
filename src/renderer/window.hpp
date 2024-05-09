#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <functional>
#include <vulkan/vulkan_raii.hpp>

namespace rendr{
//return Vulkan instance extensions required by GLFW
std::vector<const char*> getRequiredExtensions();

struct WindowData{ 
    int width;
    int height;
    std::string title;
};

struct WindowCallbacks{
    std::function<void(int, int)> winResized = [](int, int){};
    std::function<void(int)> keyDown = [](int){};
    std::function<void(int)> keyPressed = [](int){};
    std::function<void(int)> keyUp = [](int){};
    std::function<void(double, double)> mouseMoved = [](double, double){};
    std::function<void(int)> mouseBtnDown = [](int){};
    std::function<void(int)> mouseBtnUp = [](int){};
    std::function<void(double, double)> mouseScroll = [](double, double){};
    std::function<void(uint32_t)> charInput = [](uint32_t){};
};

struct GlfwWindow 
{
    GLFWwindow* window_ptr_;
    
    explicit GlfwWindow(WindowData& winData){
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        
        window_ptr_ = glfwCreateWindow(winData.width, winData.height, winData.title.data(), nullptr, nullptr);
        glfwSetWindowUserPointer(window_ptr_, this);        
    }

    GlfwWindow(const GLFWwindow& otherWin) = delete;
    GlfwWindow(GlfwWindow && otherWin) = delete;
    
    ~GlfwWindow(){
        glfwDestroyWindow(window_ptr_);
        glfwTerminate();
    }
};

class Window
{
private:
    GlfwWindow window_;
    WindowData winData_;
    
public:
    WindowCallbacks callbacks;
    
    Window( int width = 800, int height  = 800, const std::string& title = "Vulkan App")
    : winData_{width, height, title}, window_(winData_){}
    
    void setCallbacks() {

    }

    //TODO работа с ошибками
    vk::raii::SurfaceKHR createSurface(vk::raii::Instance& instance){
        VkSurfaceKHR surface;
        if (glfwCreateWindowSurface(static_cast<VkInstance>(*instance), window_.window_ptr_, nullptr, &surface) != VK_SUCCESS){
            throw std::runtime_error("failed to create window surface!");
        }
        return vk::raii::SurfaceKHR(instance, surface); 
    }

    bool shouldClose(){
        return static_cast<bool>(glfwWindowShouldClose(window_.window_ptr_));
    }

    void pollIvents(){
        glfwPollEvents();
    }  
};
}   