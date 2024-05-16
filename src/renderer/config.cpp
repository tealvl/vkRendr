#include "config.hpp"

namespace rendr{

const std::vector<const char*> DebugConfig::validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::string AppInfo::name = "App"s;
const std::string AppInfo::engineName = "Renderer"s;
const int AppInfo::version = 1;
const int AppInfo::engineVersion = 1;
const uint32_t AppInfo::apiVersion = VK_API_VERSION_1_1;



}


