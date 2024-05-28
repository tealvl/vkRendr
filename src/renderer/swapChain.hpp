#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "device.hpp"

namespace rendr{

class SwapChain{
//private:
public:
    vk::raii::SwapchainKHR swapChain_;
    vk::Format swapChainImageFormat_;
    vk::Extent2D swapChainExtent_;
    std::vector<vk::Image> swapChainImages_;
    std::vector<vk::raii::ImageView> swapChainImageViews_;
public:
    SwapChain();

    void create(const rendr::Device&  renderDevice, const rendr::Window& win);
    void clear();
};


}