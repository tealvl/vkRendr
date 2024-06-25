#pragma once
#include "rendr.hpp"

class SimpleSetupBinder : public rendr::ISetupBinder{
    rendr::RendererSetup createRendererSetup(
        const rendr::Renderer& renderer, 
        const vk::raii::DescriptorSetLayout& rendererGlobalDescriptorSetLayout,  
        int framesInFlight) override{
        
        const rendr::Device& device = renderer.getDevice();
        const rendr::SwapChain& swapChain = renderer.getSwapChain();
        const rendr::Image& depthImage = renderer.getDepthImage();

        rendr::RendererSetup setup;
        setup.renderPass_ = rendr::createRenderPassWithColorAndDepthAttOneSubpass(device.logicalDevice_, swapChain.swapChainImageFormat_, rendr::findDepthFormat(device.physicalDevice_));
        setup.descriptorSetLayout_ = rendr::createSamplerDescriptorSetLayout(device.logicalDevice_);
        setup.pipelineLayout_ = rendr::createPipelineLayout(device.logicalDevice_, {*rendererGlobalDescriptorSetLayout, *setup.descriptorSetLayout_}, {});

        std::vector<char> vertShaderCode = rendr::readFile("C:/Dev/cpp-projects/engine/src/shaders/fvertex.spv");
        std::vector<char> fragShaderCode = rendr::readFile("C:/Dev/cpp-projects/engine/src/shaders/ffragment.spv");
        vk::raii::ShaderModule vertShaderModule = rendr::createShaderModule(device.logicalDevice_, vertShaderCode);
        vk::raii::ShaderModule fragShaderModule = rendr::createShaderModule(device.logicalDevice_, fragShaderCode);

        setup.graphicsPipeline_ = rendr::createGraphicsPipelineWithDefaults(device.logicalDevice_, setup.renderPass_, setup.pipelineLayout_, swapChain.swapChainExtent_, rendr::VertexPTN{},
            vertShaderModule, fragShaderModule
        );
        setup.swapChainFramebuffers_ = rendr::createSwapChainFramebuffersWithDepthAtt(device.logicalDevice_, setup.renderPass_, swapChain.swapChainImageViews_, depthImage.imageView, swapChain.swapChainExtent_.width, swapChain.swapChainExtent_.height);

        setup.swapChainFramebuffersRecreationFunc_ = [](const rendr::Renderer& renderer, rendr::RendererSetup& setup){
            const rendr::Device& device = renderer.getDevice();
            const rendr::SwapChain& swapChain = renderer.getSwapChain();
            const rendr::Image& depthImage = renderer.getDepthImage();
            setup.swapChainFramebuffers_ = rendr::createSwapChainFramebuffersWithDepthAtt(device.logicalDevice_, setup.renderPass_, swapChain.swapChainImageViews_, depthImage.imageView, swapChain.swapChainExtent_.width, swapChain.swapChainExtent_.height);
        };

        setup.descriptorPool_ = rendr::createDescriptorPool(device.logicalDevice_, framesInFlight);
        setup.descriptorSets_ = rendr::createDescriptorSets(device.logicalDevice_, setup.descriptorPool_, setup.descriptorSetLayout_, framesInFlight);

        return setup;
    }

    void bindSetupResources(const vk::raii::Device& device, const vk::raii::CommandBuffer& buffer, const vk::raii::PipelineLayout& layout, int curFrame) override{
        
    }
};