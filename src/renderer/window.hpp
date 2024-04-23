#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <functional>

namespace rendr{

class Window
{
private:
    GLFWwindow* window_;
    int width_;
    int height_;
    std::string title_;
    std::function<void (int,int)> externalResizeCallback_;
    
    static void framebufferResizeCallback (GLFWwindow* window, int height, int weight) {
        auto thisWin = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        thisWin->framebufferResized = true;
        thisWin->externalResizeCallback_;
    }

public:
    bool framebufferResized = false;

    explicit Window(int width = 800, int height = 800, std::string title = "Vulkan",  std::function<void (int,int)> resizeCallback = [](int, int){})
        : width_(width), height_(height), title_(std::move(title)), externalResizeCallback_(std::move(resizeCallback))
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        
        window_ = glfwCreateWindow(width_, height_, title.data(), nullptr, nullptr);
        glfwSetWindowUserPointer(window_, this);
        glfwSetFramebufferSizeCallback(window_, framebufferResizeCallback);
    }

    ~Window(){
        glfwDestroyWindow(window_);
        glfwTerminate();
    }
};
}   