#pragma once
#include <vector>
#include <string>
#include <vulkan/vulkan_raii.hpp>

using namespace std::literals;
namespace rendr{

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


struct PhisycalDeviceConfig{

};


struct DeviceConfig{
    
};


}
