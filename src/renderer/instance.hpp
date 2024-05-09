#pragma once
#include <vector>
#include <vulkan/vulkan_raii.hpp>
#include <iostream>
#include "debug.hpp"
#include "window.hpp"

namespace rendr{

//TODO вынести в отдельный файл
const std::string appName = "App";
const std::string engineName = "Renderer";
const int appVersion = 1;
const int engineVersion = 1;

class Instance{
private:
    vk::raii::Context  context_;
    vk::raii::Instance instance_;
    vk::raii::DebugUtilsMessengerEXT debugUtilsMessenger_;

public:
    Instance()
    :context_(), instance_(nullptr), debugUtilsMessenger_(nullptr) 
    {   
        //TODO Подумать над обработкой ошибок    
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        vk::ApplicationInfo applicationInfo(appName.data(), appVersion, engineName.data(), engineVersion, VK_API_VERSION_1_1 );
        vk::InstanceCreateInfo instanceCreateInfo( {}, &applicationInfo );
        
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfoOutsideInstance{};
        if (enableValidationLayers) {
            instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfoOutsideInstance);
            instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfoOutsideInstance;
        } 
        else {
            instanceCreateInfo.enabledLayerCount = 0;
            instanceCreateInfo.pNext = nullptr;
        }

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;

        std::vector<const char *> extensions = rendr::getRequiredExtensions();
        
        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

        instance_ = vk::raii::Instance( context_, instanceCreateInfo );
        
        if (enableValidationLayers) {
            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfoInsideInstance{};
            populateDebugMessengerCreateInfo(debugCreateInfoOutsideInstance);
            debugUtilsMessenger_ = vk::raii::DebugUtilsMessengerEXT(instance_, debugCreateInfoInsideInstance);
        }
    }

    vk::raii::Instance const& operator*(){
        return instance_;
    }
};
}