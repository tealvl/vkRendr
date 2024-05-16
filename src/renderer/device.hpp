#pragma once
#include <vulkan/vulkan_raii.hpp>

namespace rendr{

    class Device{
    private:
        vk::raii::SurfaceKHR surface_;
        vk::raii::PhysicalDevice physicalDevice_;
        vk::raii::Device device_;
    };

}