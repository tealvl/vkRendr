#pragma once
#include <vector>
#include <vulkan/vulkan_raii.hpp>
#include <iostream>
#include "debug.hpp"
#include "window.hpp"
#include "config.hpp"

namespace rendr{
class Instance {
private:
    vk::raii::Context context_;
    vk::raii::Instance instance_;
    vk::raii::DebugUtilsMessengerEXT debugUtilsMessenger_;

public:
    Instance()
        : context_(), instance_(nullptr), debugUtilsMessenger_(nullptr) {}

    Instance(Window const & window)
        : context_(), instance_(nullptr), debugUtilsMessenger_(nullptr) {
        if (DebugConfig::enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        vk::ApplicationInfo applicationInfo(
            AppInfo::name.data(),
            AppInfo::version,
            AppInfo::engineName.data(),
            AppInfo::engineVersion,
            AppInfo::apiVersion
        );

        vk::InstanceCreateInfo instanceCreateInfo({}, &applicationInfo);

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfoOutsideInstance{};
        if (DebugConfig::enableValidationLayers) {
            instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(DebugConfig::validationLayers.size());
            instanceCreateInfo.ppEnabledLayerNames = DebugConfig::validationLayers.data();
            populateDebugMessengerCreateInfo(debugCreateInfoOutsideInstance);
            instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfoOutsideInstance;
        } else {
            instanceCreateInfo.enabledLayerCount = 0;
            instanceCreateInfo.pNext = nullptr;
        }

        std::vector<const char*> extensions = window.getRequiredExtensions();

        if (DebugConfig::enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

        instance_ = vk::raii::Instance(context_, instanceCreateInfo);

        if (DebugConfig::enableValidationLayers) {
            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfoInsideInstance{};
            populateDebugMessengerCreateInfo(debugCreateInfoInsideInstance);
            debugUtilsMessenger_ = vk::raii::DebugUtilsMessengerEXT(instance_, debugCreateInfoInsideInstance);
        }
    }

    
    Instance(Instance&& other) noexcept
        : context_(std::move(other.context_)),
          instance_(std::move(other.instance_)),
          debugUtilsMessenger_(std::move(other.debugUtilsMessenger_)) {}

    
    Instance& operator=(Instance&& other) noexcept {
        if (this != &other) {
            context_ = std::move(other.context_);
            instance_ = std::move(other.instance_);
            debugUtilsMessenger_ = std::move(other.debugUtilsMessenger_);
        }
        return *this;
    }

    
    Instance(const Instance&) = delete;
    Instance& operator=(const Instance&) = delete;

    vk::raii::Instance const& operator*() const {
        return instance_;
    }
};

}