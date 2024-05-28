#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "instance.hpp"
#include "window.hpp"
#include "utility.hpp"
#include "deviceConfig.hpp"

namespace rendr{

    class SwapChain;
    class RenderSetup;
    
    class Device{
    //private:
    public:
        rendr::Instance instance_; 
        vk::raii::SurfaceKHR surface_;
        vk::raii::PhysicalDevice physicalDevice_;
        vk::raii::Device device_;
        vk::raii::Queue graphicsQueue_;
        vk::raii::Queue presentQueue_;
        vk::raii::CommandPool commandPool_;

        friend class rendr::SwapChain;
        friend class rendr::RenderSetup;
    public:
        Device();
        void create(DeviceConfig config, const rendr::Window& win);

    };

}