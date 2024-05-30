#pragma once
#include "utility.hpp"

class MeshWithTextureObj : public rendr::IDrawableObj{
    rendr::Image texture;
    rendr::Buffer indexBuffer;
    rendr::Buffer vertexBuffer;
    vk::raii::Sampler sampler;
    vk::raii::DescriptorPool descriptorPool;
    std::vector<vk::raii::DescriptorSet> descriptorSets;
     
    size_t numOfIndices;
public:

    MeshWithTextureObj(rendr::Material& mat)
    : IDrawableObj(mat), sampler(nullptr),descriptorPool(nullptr) {}

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
        int framesOnFlight = renderer.getNumOfFramesInFlight();
        descriptorSets.clear();
        descriptorPool = rendr::createDescriptorPool(device.device_, framesOnFlight);
        const rendr::RendererSetup& setup = renderer.getRenderSetup(renderMaterial->renderSetupIndex);
        const vk::raii::DescriptorSetLayout& layout = setup.descriptorSetLayout_;
        descriptorSets = rendr::createDescriptorSets(device.device_, descriptorPool, layout, framesOnFlight);

        for(int i = 0; i < framesOnFlight; i++){
            vk::DescriptorImageInfo imageInfo(
                *sampler, // sampler
                *texture.imageView,
                vk::ImageLayout::eShaderReadOnlyOptimal // imageLayout
            );

            std::array<vk::WriteDescriptorSet, 1> descriptorWrites = {        
                vk::WriteDescriptorSet(
                    *descriptorSets[i], // dstSet
                    0, // dstBinding
                    0, // dstArrayElement
                    1, // descriptorCount
                    vk::DescriptorType::eCombinedImageSampler, // descriptorType
                    &imageInfo, // pImageInfo
                    nullptr, // pBufferInfo
                    nullptr // pTexelBufferView
                )
            };

            device.device_.updateDescriptorSets(descriptorWrites, nullptr);
        }
    }

    size_t getNumOfDrawIndices() override{
        return numOfIndices;
    }

    void bindResources(
        const vk::raii::Device& device, 
        const vk::raii::CommandBuffer& buffer, 
        const vk::raii::PipelineLayout& layout, 
        const vk::raii::DescriptorSet& rendererUboDescriptorSet, 
        int curFrame) override{
    
        buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *layout, 0, {*rendererUboDescriptorSet,*descriptorSets[curFrame]}, {});
        buffer.bindVertexBuffers(0, *vertexBuffer.buffer, {0});
        buffer.bindIndexBuffer(*indexBuffer.buffer, 0, vk::IndexType::eUint32);
    }
};