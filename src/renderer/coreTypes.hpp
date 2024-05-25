#pragma once
#include <vulkan/vulkan_raii.hpp>
#include <vector>

namespace rendr{

struct Image{
    vk::raii::ImageView imageView;
    vk::raii::DeviceMemory imageMemory;
    vk::raii::Image image;

    Image() : image(nullptr), imageMemory(nullptr), imageView(nullptr){}
};

struct Buffer{   
    vk::raii::DeviceMemory bufferMemory;
    vk::raii::Buffer buffer;
    Buffer() : buffer(nullptr), bufferMemory(nullptr){}
};

template<typename VertexType>
struct Mesh{
    std::vector<VertexType> vertices;
    std::vector<uint32_t> indices;
};


struct Material{
    uint32_t materialIndex;
    std::vector<rendr::Image> materialImages;
    std::vector<rendr::Buffer> matBuffers;
};


struct Batch{
    rendr::Buffer vertices;
    rendr::Buffer indices;
    Material* materialIndex;
};



}