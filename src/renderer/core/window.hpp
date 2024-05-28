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
    std::function<void(int)> winFocused = [](int){};
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
    WindowData data_;
    static void framebufferResizeInternalCallback(GLFWwindow* win, int width, int height);
    static void keyDownInternalCallback(GLFWwindow* win, int key, int scancode, int action, int mods);
    static void mouseMovedInternalCallback(GLFWwindow* win, double xpos, double ypos);
    static void mouseButtonInternalCallback(GLFWwindow* win, int button, int action, int mods);
    static void mouseScrollInternalCallback(GLFWwindow* win, double xoffset, double yoffset);
    static void charInputInternalCallback(GLFWwindow* win, unsigned int codepoint);
    static void windowFocusInternalCallback(GLFWwindow *window, int focused);

    void setCallbacks();
public:
    WindowCallbacks callbacks;
    
    Window( int width = 800, int height  = 800, const std::string& title = "Vulkan App")
    : window_({width, height, title}){
        data_.height_ = height;
        data_.width_ = width;
        data_.title_ = title;
        setCallbacks();
    }

    vk::raii::SurfaceKHR createSurface(const vk::raii::Instance &instance) const;

    bool shouldClose() const;

    //return <width, height> pair
    std::pair<int, int> getFramebufferSize() const;
    
    //return Vulkan instance extensions required by GLFW
    std::vector<const char*> getRequiredExtensions() const;
    
    void pollEvents() const;
    void waitEvents() const;

    void enableCursor();
    void disableCursor();
};

}   