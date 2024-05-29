#pragma once
#include "renderer.hpp"
#include "utility.hpp"

class SimpleMaterial : public rendr::Material{
    rendr::RendererSetup createRendererSetup(const rendr::Device& device, const rendr::SwapChain& swapChain) override{
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
        return setup;
    }
};