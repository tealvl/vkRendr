#include <vector>
#include <fstream>
#include <vulkan/vulkan_raii.hpp>

namespace rendr{

//TODO вынести в файл конфигурации требования к девайсу
bool isPhysicalDeviceSuitable(const vk::raii::PhysicalDevice& device);

vk::raii::PhysicalDevice pickPhysicalDevice(const vk::raii::Instance& instance);

static std::vector<char> readFile(const std::string& filename) {
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