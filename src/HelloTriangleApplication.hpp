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

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                   (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                   (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

const int MAX_FRAMES_IN_FLIGHT = 2;

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

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
    vertexBuffer_()
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
        
        
        rendr::endSingleTimeCommands(singleTimeCommandBuffer, graphicsQueue_);

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

    vk::raii::SwapchainKHR swapChain_;
    vk::Format swapChainImageFormat_;
    vk::Extent2D swapChainExtent_;
    std::vector<vk::Image> swapChainImages_;
    std::vector<vk::raii::ImageView> swapChainImageViews_;
    
    vk::raii::DescriptorSetLayout descriptorSetLayout_;
    vk::raii::RenderPass renderPass_;
    vk::raii::PipelineLayout pipelineLayout_;
    vk::raii::Pipeline graphicsPipeline_;

    rendr::Image depthImage_;

    std::vector<vk::raii::Framebuffer> swapChainFramebuffers_;
    vk::raii::CommandPool commandPool_;

    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;
    bool framebufferResized = false;

    rendr::Buffer vertexBuffer_;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    rendr::Image textureImage_;
    vk::raii::Sampler textureSampler_;

    std::vector<rendr::VertexPCT> vertices;
    std::vector<uint32_t> indices;

    rendr::Camera camera;
    rendr::Transform model_matrix;
    
    void initVulkan();
    void mainLoop();
    void cleanup();
    void cleanupSwapChain();
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
    VkImageView createImageView(VkImage image, VkFormat format,VkImageAspectFlags aspectFlags);

    void recreateSwapChain();
    void createGraphicsPipeline();
    VkShaderModule createShaderModule(const std::vector<char> &code);
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void drawFrame();
    void createSyncObjects();
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void createVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffers();
    void createDescriptorSetLayout();
    void updateUniformBuffer(uint32_t currentImage);
    void createDescriptorPool();
    void createDescriptorSets();
    void createTextureImage();
    void createTextureImageView();
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void createTextureSampler();
    void createDepthResources();
    VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat findDepthFormat();
    bool hasStencilComponent(VkFormat format);
    void loadModel();
};