#include "utility.hpp"

namespace rendr{

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}
//TODO Добавить настройку для выбора физического устройства
bool isPhysicalDeviceSuitable(vk::raii::PhysicalDevice const & device, vk::raii::SurfaceKHR const & surface) {
    vk::PhysicalDeviceFeatures features = device.getFeatures();
    vk::PhysicalDeviceProperties properties = device.getProperties();
    
    QueueFamilyIndices indices = findQueueFamilies(*device, *surface);
    bool extensionsSupported = checkDeviceExtensionSupport(*device);
    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(*device, *surface);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }
    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(*device, &supportedFeatures);

    return 
    indices.isComplete() 
    && extensionsSupported 
    && swapChainAdequate  
    && supportedFeatures.samplerAnisotropy 
    &&  properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu 
    && features.geometryShader;
}

vk::raii::PhysicalDevice pickPhysicalDevice(vk::raii::Instance const & instance, vk::raii::SurfaceKHR const & surface){
    vk::raii::PhysicalDevice physicalDevice(nullptr);
    vk::raii::PhysicalDevices devices( instance );
    
    bool suitableDevicePicked = false;
    for (const auto& device : devices) {
        if (isPhysicalDeviceSuitable(device, surface)) {
            physicalDevice = device;
            suitableDevicePicked = true;
            break;
        }
    }
    if (!suitableDevicePicked) {
        throw std::runtime_error("failed to find a suitable physical device!");
    }
    return physicalDevice;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices;
    
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if(presentSupport){
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }
        i++;
    }
    return indices;
}

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

DeviceWithQueues createDeviceWithQueues( vk::raii::PhysicalDevice const & physicalDevice,  vk::raii::SurfaceKHR const & surface){
    QueueFamilyIndices indices = findQueueFamilies(*physicalDevice, *surface);
    
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
   
    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        queueCreateInfos.push_back(vk::DeviceQueueCreateInfo( vk::DeviceQueueCreateFlags(), indices.graphicsFamily.value(), 1, &queuePriority));
    }

    //TODO вынести требования к устройству
    vk::PhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    vk::DeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.setQueueCreateInfoCount(queueCreateInfos.size()); 
    deviceCreateInfo.setPQueueCreateInfos(queueCreateInfos.data()); 
    deviceCreateInfo.setEnabledExtensionCount(deviceExtensions.size()); 
    deviceCreateInfo.setPpEnabledExtensionNames(deviceExtensions.data()); 
    deviceCreateInfo.setPEnabledFeatures(&deviceFeatures); 
    deviceCreateInfo.setFlags(vk::DeviceCreateFlags());

    vk::raii::Device device(physicalDevice, deviceCreateInfo);
    vk::raii::Queue graphicsQueue(device, indices.graphicsFamily.value(), 0);
    vk::raii::Queue presentQueue (device, indices.presentFamily.value(), 0);
    
    return DeviceWithQueues{std::move(device), std::move(graphicsQueue), std::move(presentQueue)};
}

// VkSurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> const & availableFormats) {
//     for (const auto& availableFormat : availableFormats) {
//         if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
//             return availableFormat;
//         }
//     }

//     return availableFormats[0];
// }

// VkPresentModeKHR chooseSwapPresentMode(std::vector<VkPresentModeKHR> const & availablePresentModes) {
//     for (const auto& availablePresentMode : availablePresentModes) {
//         if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
//             return availablePresentMode;
//         }
//     }

//     return VK_PRESENT_MODE_FIFO_KHR;
// }

// VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR const & capabilities) {
//     if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
//         return capabilities.currentExtent;
//     } else {
//         auto size = window_.getFramebufferSize();
//         int width = size.first;
//         int height = size.second;
        
//         VkExtent2D actualExtent = {
//             static_cast<uint32_t>(width),
//             static_cast<uint32_t>(height)
//         };

//         actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
//         actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

//         return actualExtent;
//     }
// }

// vk::SwapchainKHR createSwapChain(vk::raii::PhysicalDevice const & physicalDevice, vk::raii::SurfaceKHR const & surface, vk::raii::Device const & device) 
// {
//     VkSwapchainKHR swapChain;
//     std::vector<VkImage> swapChainImages;

//     rendr::SwapChainSupportDetails swapChainSupport = rendr::querySwapChainSupport(*physicalDevice, *surface);

//     VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
//     VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
//     VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
//     uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1; 

    
//     if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
//         imageCount = swapChainSupport.capabilities.maxImageCount;
//     }
//     VkSwapchainCreateInfoKHR createInfo{};
//     createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
//     createInfo.surface = *surface;

//     createInfo.minImageCount = imageCount;
//     createInfo.imageFormat = surfaceFormat.format;
//     createInfo.imageColorSpace = surfaceFormat.colorSpace;
//     createInfo.imageExtent = extent;
//     createInfo.imageArrayLayers = 1;
//     createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

//     rendr::QueueFamilyIndices indices = rendr::findQueueFamilies(*physicalDevice, *surface);
//     uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

//     if (indices.graphicsFamily != indices.presentFamily) {
//         createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
//         createInfo.queueFamilyIndexCount = 2;
//         createInfo.pQueueFamilyIndices = queueFamilyIndices;
//     } else {
//         createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
//         createInfo.queueFamilyIndexCount = 0;
//         createInfo.pQueueFamilyIndices = nullptr;
//     }
//     createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
//     createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
//     createInfo.presentMode = presentMode;
//     createInfo.clipped = VK_TRUE;
//     createInfo.oldSwapchain = VK_NULL_HANDLE;

//     if (vkCreateSwapchainKHR(*device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
//         throw std::runtime_error("failed to create swap chain!");
//     }

//     vkGetSwapchainImagesKHR(*device, swapChain, &imageCount, nullptr);
//     swapChainImages.resize(imageCount);
//     vkGetSwapchainImagesKHR(*device, swapChain, &imageCount, swapChainImages.data());

//     VkFormat swapChainImageFormat;
//     VkExtent2D swapChainExtent;

//     swapChainImageFormat = surfaceFormat.format;
//     swapChainExtent = extent;
// }

}