#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <functional>
#include <vulkan/vulkan_raii.hpp>

namespace rendr{

struct WindowData{ 
    int width_;
    int height_;
    std::string title_;

    WindowData() : width_(0), height_(0), title_("") {}

    WindowData(int width, int height, const std::string& title)
        : width_(width), height_(height), title_(title) {}

    WindowData(const WindowData& other)
        : width_(other.width_), height_(other.height_), title_(other.title_) {}
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
    GlfwWindow()
    : window_ptr_(nullptr){} 

    GlfwWindow(WindowData winData){
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
        if(window_ptr_ != nullptr){
            glfwDestroyWindow(window_ptr_);
        }
    }
};

class Window
{
private:
    GlfwWindow window_;

    static void framebufferResizeInternalCallback(GLFWwindow* win, int width, int height);
    void setCallbacks();
public:
    WindowCallbacks callbacks;
    
    Window( int width = 800, int height  = 800, const std::string& title = "Vulkan App")
    : window_({width, height, title}){
        setCallbacks();
    }
    
    vk::raii::SurfaceKHR createSurface(const vk::raii::Instance& instance) const;
    
    bool shouldClose() const;

    //return <width, height> pair
    std::pair<int, int> getFramebufferSize() const;
    
    //return Vulkan instance extensions required by GLFW
    std::vector<const char*> getRequiredExtensions() const;
    
    void pollEvents() const;
};

}   