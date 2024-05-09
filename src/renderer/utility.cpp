#pragma once
#include "utility.hpp"

namespace rendr{


bool isPhysicalDeviceSuitable(const vk::raii::PhysicalDevice& device) {
    vk::PhysicalDeviceFeatures features = device.getFeatures();
    vk::PhysicalDeviceProperties properties = device.getProperties();
  
    return properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu &&
        features.geometryShader;
}


vk::raii::PhysicalDevice pickPhysicalDevice(const vk::raii::Instance& instance){
    vk::raii::PhysicalDevice physicalDevice(nullptr);
    vk::raii::PhysicalDevices devices( instance );
    
    bool suitableDevicePicked = false;
    for (const auto& device : devices) {
        if (isPhysicalDeviceSuitable(device)) {
            physicalDevice = device;
            suitableDevicePicked = true;
            break;
        }
    }
    //TODO ошибки
    if (!suitableDevicePicked) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
    return physicalDevice;
}

}