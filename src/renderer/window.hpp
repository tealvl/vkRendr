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
    int width_;
    int height_;
    std::string title_;
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


struct GlfwContext{
    GlfwContext(){
        glfwInit();
    }

    ~GlfwContext(){
        glfwTerminate();
    }
};

class GlfwWindow 
{
private:
    GLFWwindow* window_ptr_;
public:
    GlfwWindow(WindowData& winData){
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        
        window_ptr_ = glfwCreateWindow(winData.width_, winData.height_, winData.title_.data(), nullptr, nullptr);
        glfwSetWindowUserPointer(window_ptr_, this);        
    }

    GlfwWindow(const GLFWwindow& otherWin) = delete;
    GlfwWindow(GlfwWindow&& otherWin) noexcept : window_ptr_(nullptr) {
        *this = std::move(otherWin);
    }

    GlfwWindow& operator=(const GlfwWindow& otherWin) = delete;
    GlfwWindow& operator=(GlfwWindow&& otherWin) noexcept {
        if(this != &otherWin){
            std::swap(window_ptr_, otherWin.window_ptr_);
        }
        return *this;
    }
    
    GLFWwindow* const & operator*() const{
        return window_ptr_;
    }
   
    ~GlfwWindow(){
        glfwDestroyWindow(window_ptr_);
    }
};

class Window
{
private:
    GlfwWindow window_;
    WindowData winData_;
    
public:
    WindowCallbacks callbacks_;
    
    Window( int width = 800, int height  = 800, const std::string& title = "Vulkan App")
    : winData_{width, height, title}, window_(winData_){}
    
    void setCallbacks() {

    }

    //TODO работа с ошибками
    vk::raii::SurfaceKHR createSurface(const vk::raii::Instance& instance){
        VkSurfaceKHR surface;
        if (glfwCreateWindowSurface(static_cast<VkInstance>(*instance), *window_, nullptr, &surface) != VK_SUCCESS){
            throw std::runtime_error("failed to create window surface!");
        }
        return vk::raii::SurfaceKHR(instance, surface); 
    }

    bool shouldClose(){
        return static_cast<bool>(glfwWindowShouldClose(*window_));
    }

    void pollEvents(){
        glfwPollEvents();
    }  
};
}   