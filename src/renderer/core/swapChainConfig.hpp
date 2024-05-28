#pragma once
#include <vector>
#include <string>
#include <vulkan/vulkan_raii.hpp>
#include <functional>

namespace rendr{

struct SwapChainConfig{
    std::function<bool(vk::SurfaceFormatKHR)> isSurfaceFormatSuitable = [](vk::SurfaceFormatKHR format){
        return format.format == vk::Format::eR8G8B8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
    };

    std::function<bool(vk::PresentModeKHR)> isPresentModeSuitable = [](vk::PresentModeKHR presentMode){
        return presentMode == vk::PresentModeKHR::eMailbox;
    };
};


}