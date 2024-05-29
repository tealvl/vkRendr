#pragma once
#include "utility.hpp"

class SimpleMaterial : public rendr::Material{
    rendr::RendererSetup createRendererSetup(const rendr::Renderer& renderer) override{
        
        const rendr::Device& device = renderer.getDevice();
        const rendr::SwapChain& swapChain = renderer.getSwapChain();
        const rendr::Image& depthImage = renderer.getDepthImage();

        rendr::RendererSetup setup;
        setup.renderPass_ = rendr::createRenderPassWithColorAndDepthAttOneSubpass(device.device_, swapChain.swapChainImageFormat_, rendr::findDepthFormat(device.physicalDevice_));
        setup.descriptorSetLayout_ = rendr::createUboAndSamplerDescriptorSetLayout(device.device_);
        setup.pipelineLayout_ = rendr::createPipelineLayout(device.device_, {*setup.descriptorSetLayout_}, {});

        std::vector<char> vertShaderCode = rendr::readFile("C:/Dev/cpp-projects/engine/src/shaders/fvertex.spv");
        std::vector<char> fragShaderCode = rendr::readFile("C:/Dev/cpp-projects/engine/src/shaders/ffragment.spv");
        vk::raii::ShaderModule vertShaderModule = rendr::createShaderModule(device.device_, vertShaderCode);
        vk::raii::ShaderModule fragShaderModule = rendr::createShaderModule(device.device_, fragShaderCode);

        setup.graphicsPipeline_ = rendr::createGraphicsPipelineWithDefaults(device.device_, setup.renderPass_, setup.pipelineLayout_, swapChain.swapChainExtent_, rendr::VertexPTN{},
            vertShaderModule, fragShaderModule
        );
        setup.swapChainFramebuffers_ = rendr::createSwapChainFramebuffersWithDepthAtt(device.device_, setup.renderPass_, swapChain.swapChainImageViews_, depthImage.imageView, swapChain.swapChainExtent_.width, swapChain.swapChainExtent_.height);

        setup.swapChainFramebuffersRecreationFunc_ = [&setup](const rendr::Renderer& renderer){
            const rendr::Device& device = renderer.getDevice();
            const rendr::SwapChain& swapChain = renderer.getSwapChain();
            const rendr::Image& depthImage = renderer.getDepthImage();
            setup.swapChainFramebuffers_ = rendr::createSwapChainFramebuffersWithDepthAtt(device.device_, setup.renderPass_, swapChain.swapChainImageViews_, depthImage.imageView, swapChain.swapChainExtent_.width, swapChain.swapChainExtent_.height);
        };

        return setup;
    }
};