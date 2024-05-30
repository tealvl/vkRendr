#pragma once
#include "utility.hpp"

class MeshWithTextureObj : public rendr::IDrawableObj{
    rendr::Image texture;
    rendr::Buffer indexBuffer;
    rendr::Buffer vertexBuffer;
    vk::raii::Sampler sampler;
    size_t numOfIndices;
public:

    MeshWithTextureObj(rendr::Material& mat)
    : IDrawableObj(mat), sampler(nullptr){}

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

    void bindResources(const vk::raii::Device& device, const vk::raii::CommandBuffer& buffer, const vk::raii::DescriptorSet& dstDescrSet) override{
        
        vk::DescriptorImageInfo imageInfo(
            *sampler, // sampler
            *texture.imageView,
            vk::ImageLayout::eShaderReadOnlyOptimal // imageLayout
        );

        std::array<vk::WriteDescriptorSet, 1> descriptorWrites = {        
            vk::WriteDescriptorSet(
                *dstDescrSet, // dstSet
                1, // dstBinding
                0, // dstArrayElement
                1, // descriptorCount
                vk::DescriptorType::eCombinedImageSampler, // descriptorType
                &imageInfo, // pImageInfo
                nullptr, // pBufferInfo
                nullptr // pTexelBufferView
            )
        };

        device.updateDescriptorSets(descriptorWrites, nullptr);

        buffer.bindVertexBuffers(0, *vertexBuffer.buffer, {0});
        buffer.bindIndexBuffer(*indexBuffer.buffer, 0, vk::IndexType::eUint32);
    }
};