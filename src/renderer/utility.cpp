#include "utility.hpp"
#include <limits>
#include <algorithm>
#include "window.hpp"

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
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
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

SwapChainSupportDetails querySwapChainSupport(vk::raii::PhysicalDevice const & device, vk::raii::SurfaceKHR const & surface) {
    SwapChainSupportDetails details;
    details.capabilities = device.getSurfaceCapabilitiesKHR(*surface);
    details.formats = device.getSurfaceFormatsKHR(*surface);
    details.presentModes = device.getSurfacePresentModesKHR(*surface);
    
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

VkSurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const & availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const & availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            return availablePresentMode;
        }
    }

    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const & capabilities, std::pair<int,int> const & winFramebufferSize) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        vk::Extent2D actualExtent = {
            static_cast<uint32_t>(winFramebufferSize.first),
            static_cast<uint32_t>(winFramebufferSize.second)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

SwapChainData createSwapChain(vk::raii::PhysicalDevice const & physicalDevice, vk::raii::SurfaceKHR const & surface, vk::raii::Device const & device, rendr::Window const & win) 
{
    rendr::SwapChainSupportDetails swapChainSupport = rendr::querySwapChainSupport(physicalDevice, surface);
    vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities, win.getFramebufferSize());
    
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1; 
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    
    vk::SwapchainCreateInfoKHR createInfo{};
    createInfo.surface = *surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

    rendr::QueueFamilyIndices indices = rendr::findQueueFamilies(*physicalDevice, *surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    vk::raii::SwapchainKHR swapChain(device, createInfo);

    return {std::move(swapChain), surfaceFormat.format, extent};
}

// aspect flags  -  VK_IMAGE_ASPECT_COLOR_BIT 
vk::raii::ImageView createImageView(vk::raii::Device const & device, vk::Image const & image, vk::Format const & format, vk::ImageAspectFlags aspectFlags) {
    vk::ImageViewCreateInfo viewCreateInfo{};
    viewCreateInfo.image = image;
    viewCreateInfo.viewType = vk::ImageViewType::e2D;
    viewCreateInfo.format = format;
    viewCreateInfo.subresourceRange.aspectMask = aspectFlags;
    viewCreateInfo.subresourceRange.baseMipLevel = 0;
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    viewCreateInfo.subresourceRange.layerCount = 1;

    vk::raii::ImageView imageView(device, viewCreateInfo);

    return imageView;
}

std::vector<vk::raii::ImageView> createImageViews(std::vector<vk::Image> const & images, vk::raii::Device const & device, vk::Format const & format, vk::ImageAspectFlags aspectFlags)
{
    std::vector<vk::raii::ImageView> swapChainImageViews;
    swapChainImageViews.reserve(images.size());
    for (auto & image : images) {
        
       swapChainImageViews.push_back(createImageView(device, image, format, aspectFlags));
    }
    return swapChainImageViews;
}

}