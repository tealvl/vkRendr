#pragma once
#include <vector>
#include <fstream>
#include <set>
#include <string>
#include <vulkan/vulkan_raii.hpp>
#include <optional>
#include "window.hpp"

namespace rendr{

//TODO вынести в файл конфигурации требования к девайсу
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct QueueFamilyIndices {
std::optional<uint32_t> graphicsFamily;
std::optional<uint32_t> presentFamily;

bool isComplete() {
    return graphicsFamily.has_value() && presentFamily.has_value();
}
};

struct DeviceWithQueues{
    vk::raii::Device device;
    vk::raii::Queue graphicsQueue;
    vk::raii::Queue presentQueue;
};

struct SwapChainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

struct SwapChainData{
    vk::raii::SwapchainKHR swapChain;
    vk::Format swapChainImageFormat;
    vk::Extent2D swapChainExtent;
};

bool checkDeviceExtensionSupport(VkPhysicalDevice device);
bool isPhysicalDeviceSuitable(vk::raii::PhysicalDevice const &device, vk::raii::SurfaceKHR const &surface);
vk::raii::PhysicalDevice pickPhysicalDevice(vk::raii::Instance const &instance, vk::raii::SurfaceKHR const &surface);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
SwapChainSupportDetails querySwapChainSupport(vk::raii::PhysicalDevice const &device, vk::raii::SurfaceKHR const &surface);
DeviceWithQueues createDeviceWithQueues(vk::raii::PhysicalDevice const &physicalDevice, vk::raii::SurfaceKHR const &surface);

VkSurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const &availableFormats);

vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &availablePresentModes);

vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities, std::pair<int, int> const &winFramebufferSize);

SwapChainData createSwapChain(vk::raii::PhysicalDevice const &physicalDevice, vk::raii::SurfaceKHR const &surface, vk::raii::Device const &device, Window const &win);

vk::raii::ImageView createImageView(vk::raii::Device const &device, vk::Image const &image, vk::Format const &format, vk::ImageAspectFlags aspectFlags);

std::vector<vk::raii::ImageView> createImageViews(std::vector<vk::Image> const &images, vk::raii::Device const &device, vk::Format const &format, vk::ImageAspectFlags aspectFlags);

static std::vector<char> readFile(std::string const &filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}
}