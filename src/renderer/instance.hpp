#pragma once
#include <vector>
#include <vulkan/vulkan_raii.hpp>
#include <iostream>

#include "window.hpp"

namespace rendr{

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                    void* pUserData
);
bool checkValidationLayerSupport();
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);


struct DebugConfig{   
#ifdef NDEBUG
    static const bool enableValidationLayers = false;
#else
    static const bool enableValidationLayers = true;
#endif
    static const std::vector<const char*> validationLayers;
};

struct AppInfo{
    static const std::string name;
    static const std::string engineName;
    static const int version;
    static const int engineVersion;
    static const uint32_t apiVersion;
};

class Instance {
private:
    vk::raii::Context context_;
    vk::raii::Instance instance_;
    vk::raii::DebugUtilsMessengerEXT debugUtilsMessenger_;

public:
    Instance();
    Instance(Window const & window);

    Instance(const Instance&) = delete;
    Instance& operator=(const Instance&) = delete;

    Instance(Instance&& other) noexcept;

    Instance& operator=(Instance&& other) noexcept;

    vk::raii::Instance const& operator*() const; 
};

}