#pragma once
#include "utility.hpp"

class MeshWithTextureObj : public rendr::DrawableObj{
    rendr::Image texture;
    rendr::Buffer indexBuffer;
    rendr::Buffer vertexBuffer;
    vk::raii::Sampler sampler;
    size_t numOfIndices;
public:

    MeshWithTextureObj(rendr::Material& mat)
    : DrawableObj(mat), sampler(nullptr){}

    void loadMesh(rendr::Mesh<rendr::VertexPTN>& mesh, const rendr::Renderer& renderer){
        const rendr::Device& device = renderer.getDevice();
        vertexBuffer = rendr::createVertexBuffer(device.physicalDevice_, device.device_, device.commandPool_,device.graphicsQueue_, mesh.vertices);
        indexBuffer = rendr::createIndexBuffer(device.physicalDevice_, device.device_, device.commandPool_,device.graphicsQueue_, mesh.indices);
        numOfIndices = mesh.indices.size();
    }

    void loadTexture(rendr::STBImageRaii tex, const rendr::Renderer& renderer){
        const rendr::Device& device = renderer.getDevice();
        texture = rendr::create2DTextureImage(device.physicalDevice_,device.device_, device.commandPool_, device.graphicsQueue_, std::move(tex));
        sampler = rendr::createTextureSampler(device.device_, device.physicalDevice_);
    }

    size_t getNumOfDrawIndices() override{
        return numOfIndices;
    }

    void bindResources(const vk::raii::CommandBuffer& buffer) override{
        buffer.bindVertexBuffers(0, *vertexBuffer.buffer, {0});
        buffer.bindIndexBuffer(*indexBuffer.buffer, 0, vk::IndexType::eUint32);
    }
};