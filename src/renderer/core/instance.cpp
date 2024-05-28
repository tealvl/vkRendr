#include "instance.hpp" 

namespace rendr{

const std::string AppInfo::name = "My Application";
const std::string AppInfo::engineName = "My Engine";
const int AppInfo::version = 1; 
const int AppInfo::engineVersion = 1; 
const uint32_t AppInfo::apiVersion = VK_API_VERSION_1_3; 

const std::vector<const char*> DebugConfig::validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                    void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

bool checkValidationLayerSupport(){
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : DebugConfig::validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr; 
}

Instance::Instance() : context_(), instance_(nullptr), debugUtilsMessenger_(nullptr) {}

Instance::Instance(Window const &window) : context_(), instance_(nullptr), debugUtilsMessenger_(nullptr)
{
    if (DebugConfig::enableValidationLayers && !checkValidationLayerSupport())
    {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    vk::ApplicationInfo applicationInfo(
        AppInfo::name.data(),
        AppInfo::version,
        AppInfo::engineName.data(),
        AppInfo::engineVersion,
        AppInfo::apiVersion);

    vk::InstanceCreateInfo instanceCreateInfo({}, &applicationInfo);

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfoOutsideInstance{};
    if (DebugConfig::enableValidationLayers)
    {
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(DebugConfig::validationLayers.size());
        instanceCreateInfo.ppEnabledLayerNames = DebugConfig::validationLayers.data();
        populateDebugMessengerCreateInfo(debugCreateInfoOutsideInstance);
        instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfoOutsideInstance;
    }
    else
    {
        instanceCreateInfo.enabledLayerCount = 0;
        instanceCreateInfo.pNext = nullptr;
    }

    std::vector<const char *> extensions = window.getRequiredExtensions();

    if (DebugConfig::enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

    instance_ = vk::raii::Instance(context_, instanceCreateInfo);

    if (DebugConfig::enableValidationLayers)
    {
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfoInsideInstance{};
        populateDebugMessengerCreateInfo(debugCreateInfoInsideInstance);
        debugUtilsMessenger_ = vk::raii::DebugUtilsMessengerEXT(instance_, debugCreateInfoInsideInstance);
    }
}
Instance::Instance(Instance &&other) noexcept
: context_(std::move(other.context_)),
    instance_(std::move(other.instance_)),
    debugUtilsMessenger_(std::move(other.debugUtilsMessenger_)) 
{}

Instance &Instance::operator=(Instance &&other) noexcept{
    if (this != &other) {
        context_ = std::move(other.context_);
        instance_ = std::move(other.instance_);
        debugUtilsMessenger_ = std::move(other.debugUtilsMessenger_);
    }
    return *this;
}
vk::raii::Instance const &Instance::operator*() const{
    return instance_;
}
}