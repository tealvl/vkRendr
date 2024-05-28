#pragma once
#include <vector>
#include <string>
#include <vulkan/vulkan_raii.hpp>
#include <functional>

namespace rendr{

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    std::optional<uint32_t> transferFamily;
    std::optional<uint32_t> computeFamily;

    bool isGraphicsAndPresent() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct DeviceConfig{
    std::vector<const char*> requiredDeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    vk::PhysicalDeviceFeatures deviceEnableFeatures;

    std::function<bool(vk::PhysicalDeviceFeatures)> isDeviceFeaturesSuitable = [](vk::PhysicalDeviceFeatures features){
        return features.samplerAnisotropy && features.geometryShader;
    };

    std::function<bool(vk::PhysicalDeviceProperties)> isDevicePropertiesSuitable = [](vk::PhysicalDeviceProperties properties){
        return properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu;
    };

    std::function<bool(QueueFamilyIndices)> isDeviceFamilyIndicesSuitable = [](QueueFamilyIndices familyIndices){
        return familyIndices.isGraphicsAndPresent();
    };
    
};
}

//  vk::PhysicalDeviceFeatures deviceFeatures{};
//     deviceFeatures.samplerAnisotropy = VK_TRUE;

// && features.samplerAnisotropy 
// &&  properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu 
// && features.geometryShader;