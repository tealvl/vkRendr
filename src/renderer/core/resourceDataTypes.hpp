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



template<typename BatchMaterial>
struct Batch{
    rendr::Buffer vertices;
    rendr::Buffer indices;
    BatchMaterial* materialPtr;

    Batch(rendr::Buffer&& vert, rendr::Buffer&& ind, BatchMaterial* matIndex)
        : vertices(std::move(vert)), indices(std::move(ind)), materialPtr(matIndex) {}
};

struct SimpleMaterial{
    uint32_t materialIndex;
    rendr::Image colorTexture;

    SimpleMaterial(uint32_t index, rendr::Image&& texture)
        : materialIndex(index), colorTexture(std::move(texture)) {}

};


class IDrawable{
    IMaterial mat;
};

class IMaterial{

};

class DrawableObj{
    IDrawable obj;
};



}