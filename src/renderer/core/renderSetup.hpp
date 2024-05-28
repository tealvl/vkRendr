#pragma once
#include <vulkan/vulkan_raii.hpp>
#include <vector>
#include "device.hpp"
#include "swapChain.hpp"

namespace rendr{
class RendererSetup{
private:
    vk::raii::DescriptorSetLayout descriptorSetLayout_;
    vk::raii::RenderPass renderPass_;
    vk::raii::PipelineLayout pipelineLayout_;
    vk::raii::Pipeline graphicsPipeline_;
    std::vector<vk::raii::Framebuffer> swapChainFramebuffers_;
public:
    RendererSetup();
    void createDefaultSetup(const rendr::Device& device, const rendr::SwapChain& SwapChain);
};
}