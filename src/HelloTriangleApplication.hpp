#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <optional>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <cstring>
#include <set>
#include <algorithm>
#include <limits>
#include <glm/glm.hpp>
#include <array>
#include <chrono>
#include <tiny_obj_loader.h>
#include <unordered_map>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "camera.hpp"
#include "transform.hpp"
#include "utility.hpp"
#include "instance.hpp"

const std::string MODEL_PATH = "C:/Dev/cpp-projects/engine/resources/models/vikingRoom.obj";
const std::string TEXTURE_PATH = "C:/Dev/cpp-projects/engine/resources/textures/viking_room.png";
const int FramesInFlight = 2;

class Application {
public:
    void run();

    Application()
    : 
    glwfContext_(), 
    window_(), 
    instance_(), 
    surface_(nullptr), 
    physicalDevice_(nullptr),
    device_(nullptr),
    graphicsQueue_(nullptr),
    presentQueue_(nullptr),
    swapChain_(nullptr),
    swapChainImages_(),
    renderPass_(nullptr),
    descriptorSetLayout_(nullptr),
    pipelineLayout_(nullptr),
    graphicsPipeline_(nullptr),
    depthImage_(),
    commandPool_(nullptr),
    textureImage_(),
    textureSampler_(nullptr),
    vertexBuffer_(),
    descriptorPool_(nullptr)
    {
        instance_ = rendr::Instance(window_);
        surface_ = window_.createSurface(*instance_);
        physicalDevice_ = rendr::pickPhysicalDevice(*instance_, surface_);
        
        rendr::DeviceWithQueues deviceAndQueues = rendr::createDeviceWithQueues(physicalDevice_, surface_);
        device_ = std::move(deviceAndQueues.device);
        graphicsQueue_ = std::move(deviceAndQueues.graphicsQueue);
        presentQueue_ = std::move(deviceAndQueues.presentQueue);
        
        rendr::SwapChainData swapChainData = rendr::createSwapChain(physicalDevice_, surface_, device_, window_);
        swapChain_ = std::move(swapChainData.swapChain);
        swapChainExtent_ = std::move(swapChainData.swapChainExtent);
        swapChainImageFormat_ = std::move(swapChainData.swapChainImageFormat);
        swapChainImages_ = swapChain_.getImages();
        swapChainImageViews_ = rendr::createImageViews(swapChainImages_, device_, swapChainImageFormat_, vk::ImageAspectFlagBits::eColor);

        renderPass_ = rendr::createRenderPassWithColorAndDepthAttOneSubpass(device_, swapChainImageFormat_, rendr::findDepthFormat(physicalDevice_));
        descriptorSetLayout_ = rendr::createUboAndSamplerDescriptorSetLayout(device_);
        pipelineLayout_ = rendr::createPipelineLayout(device_, {*descriptorSetLayout_}, {});
        graphicsPipeline_ = rendr::createGraphicsPipelineWithDefaults(device_, renderPass_, pipelineLayout_, swapChainExtent_);

        depthImage_ = rendr::createDepthImage(physicalDevice_, device_, swapChainExtent_.width, swapChainExtent_.height);
        swapChainFramebuffers_ = rendr::createSwapChainFramebuffersWithDepthAtt(device_, renderPass_, swapChainImageViews_, depthImage_.imageView, swapChainExtent_.width, swapChainExtent_.height);
        commandPool_ =  rendr::createGraphicsCommandPool(device_, rendr::findQueueFamilies(*physicalDevice_, *surface_));
        
        auto vertsAndInds = rendr::loadModel(MODEL_PATH);
        vertices = std::move(vertsAndInds.first);
        indices = std::move(vertsAndInds.second);

        textureSampler_ = rendr::createTextureSampler(device_, physicalDevice_);

        vk::raii::CommandBuffer singleTimeCommandBuffer = rendr::beginSingleTimeCommands(device_, commandPool_);
            rendr::STBImage imageData(TEXTURE_PATH);
            textureImage_ = rendr::create2DTextureImage(physicalDevice_, device_, singleTimeCommandBuffer, std::move(imageData));
            vertexBuffer_ = rendr::createVertexBuffer(physicalDevice_, device_, singleTimeCommandBuffer, vertices);
            indexBuffer_ = rendr::createIndexBuffer(physicalDevice_, device_, singleTimeCommandBuffer, indices);
        rendr::endSingleTimeCommands(singleTimeCommandBuffer, graphicsQueue_);
        
        uniformBuffers_ = rendr::createAndMapUniformBuffers(physicalDevice_, device_, uniformBuffersMapped_, FramesInFlight, rendr::MVPUniformBufferObject());
        descriptorPool_ = rendr::createDescriptorPool(device_, FramesInFlight);
        descriptorSets_ = rendr::createUboAndSamplerDescriptorSets(device_, descriptorPool_, descriptorSetLayout_, uniformBuffers_,
            textureSampler_, textureImage_.imageView, FramesInFlight, rendr::MVPUniformBufferObject()
        );
        commandBuffers_ = rendr::createCommandBuffers(device_, commandPool_, FramesInFlight);
        framesSyncObjs_ = rendr::createSyncObjects(device_, FramesInFlight);
    }

private:
    rendr::GlfwContext glwfContext_;
    rendr::Window window_;
    
    rendr::Instance instance_; 
    vk::raii::SurfaceKHR surface_;
    vk::raii::PhysicalDevice physicalDevice_;
    vk::raii::Device device_;

    vk::raii::Queue graphicsQueue_;
    vk::raii::Queue presentQueue_;

    vk::raii::CommandPool commandPool_;

    vk::raii::SwapchainKHR swapChain_;
    vk::Format swapChainImageFormat_;
    vk::Extent2D swapChainExtent_;
    std::vector<vk::Image> swapChainImages_;
    std::vector<vk::raii::ImageView> swapChainImageViews_;
    std::vector<vk::raii::Framebuffer> swapChainFramebuffers_;

    vk::raii::DescriptorSetLayout descriptorSetLayout_;
    vk::raii::RenderPass renderPass_;
    vk::raii::PipelineLayout pipelineLayout_;
    vk::raii::Pipeline graphicsPipeline_;

    rendr::Image depthImage_;

    std::vector<vk::raii::CommandBuffer> commandBuffers_;

    std::vector<rendr::PerFrameSync> framesSyncObjs_;

    uint32_t currentFrame = 0;
    bool framebufferResized = false;

    rendr::Buffer vertexBuffer_;
    rendr::Buffer indexBuffer_;

    std::vector<rendr::Buffer> uniformBuffers_;
    std::vector<void*> uniformBuffersMapped_;

    vk::raii::DescriptorPool descriptorPool_;
    std::vector<vk::raii::DescriptorSet> descriptorSets_;
    
    rendr::Image textureImage_;
    vk::raii::Sampler textureSampler_;

    std::vector<rendr::VertexPCT> vertices;
    std::vector<uint32_t> indices;

    rendr::Camera camera;
    rendr::Transform model_matrix;
    
    void mainLoop();
    void cleanup();
    void cleanupSwapChain();
    void recreateSwapChain();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void drawFrame();
    void updateUniformBuffer(uint32_t currentImage);
};