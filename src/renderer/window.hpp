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
    
    static void internalFramebufferResizeCallback (GLFWwindow* window, int height, int weight) {
        auto thisWin = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        thisWin->framebufferResized = true;
        thisWin->callbacks.winResized(height, weight);
    }

public:
    bool framebufferResized = false;

    explicit Window(int width = 800, int height = 800, std::string title = "Vulkan")
        : width_(width), height_(height), title_(std::move(title))
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        
        window_ = glfwCreateWindow(width_, height_, title.data(), nullptr, nullptr);
        glfwSetWindowUserPointer(window_, this);
        glfwSetFramebufferSizeCallback(window_, internalFramebufferResizeCallback);
    }

    ~Window(){
        glfwDestroyWindow(window_);
        glfwTerminate();
    }


    void setCallbacks() {

    }

    struct {
            std::function<void(int, int)> winResized = [](int, int){};
            std::function<void(int)> keyDown = [](int){};
            std::function<void(int)> keyPressed = [](int){};
            std::function<void(int)> keyUp = [](int){};
            std::function<void(double, double)> mouseMoved = [](double, double){};
            std::function<void(int)> mouseBtnDown = [](int){};
            std::function<void(int)> mouseBtnUp = [](int){};
            std::function<void(double, double)> mouseScroll = [](double, double){};
            std::function<void(uint32_t)> charInput = [](uint32_t){};
        } callbacks;
};
}   