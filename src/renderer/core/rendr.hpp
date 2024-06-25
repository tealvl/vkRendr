#pragma once

#include <vector>
#include <fstream>
#include <set>
#include <string>
#include <vulkan/vulkan_raii.hpp>
#include <tuple>
#include <iostream>
#include <optional>
#include <unordered_map>
#include <limits>
#include <algorithm>
#include <glm/glm.hpp>
#include <map>

#include "vertex.hpp"
#include "window.hpp"
#include "stb_image.h"
#include "ufbx.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace std {
    template<> struct hash<rendr::VertexPTC> {
        size_t operator()(rendr::VertexPTC const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                   (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                   (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

namespace rendr{


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                    void* pUserData
);
bool checkValidationLayerSupport();
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);


struct DebugConfig{   
#ifdef NDEBUG
    static const bool enableValidationLayers = false;
#else
    static const bool enableValidationLayers = true;
#endif
    static const std::vector<const char*> validationLayers;
};

struct AppInfo{
    static const std::string name;
    static const std::string engineName;
    static const int version;
    static const int engineVersion;
    static const uint32_t apiVersion;
};

class Instance {
private:
    vk::raii::Context context_;
    vk::raii::Instance instance_;
    vk::raii::DebugUtilsMessengerEXT debugUtilsMessenger_;

public:
    Instance();
    Instance(Window const & window);

    Instance(const Instance&) = delete;
    Instance& operator=(const Instance&) = delete;

    Instance(Instance&& other) noexcept;

    Instance& operator=(Instance&& other) noexcept;

    vk::raii::Instance const& operator*() const; 
};

struct SwapChainConfig{
    std::function<bool(vk::SurfaceFormatKHR)> isSurfaceFormatSuitable = [](vk::SurfaceFormatKHR format){
        return format.format == vk::Format::eR8G8B8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
    };

    std::function<bool(vk::PresentModeKHR)> isPresentModeSuitable = [](vk::PresentModeKHR presentMode){
        return presentMode == vk::PresentModeKHR::eMailbox;
    };
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    std::optional<uint32_t> transferFamily;
    std::optional<uint32_t> computeFamily;

    bool isGraphicsAndPresent() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct DeviceConfig{
    std::vector<const char*> requiredDeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    vk::PhysicalDeviceFeatures deviceEnableFeatures;

    std::function<bool(vk::PhysicalDeviceFeatures)> isDeviceFeaturesSuitable = [](vk::PhysicalDeviceFeatures features){
        return features.samplerAnisotropy && features.geometryShader;
    };

    std::function<bool(vk::PhysicalDeviceProperties)> isDevicePropertiesSuitable = [](vk::PhysicalDeviceProperties properties){
        return properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu;
    };

    std::function<bool(QueueFamilyIndices)> isDeviceFamilyIndicesSuitable = [](QueueFamilyIndices familyIndices){
        return familyIndices.isGraphicsAndPresent();
    };
    
};

class Device;
class SwapChain;
class RendererSetup;
class Renderer;

struct Device{
    rendr::Instance instance_; 
    vk::raii::SurfaceKHR surface_;
    vk::raii::PhysicalDevice physicalDevice_;
    vk::raii::Device logicalDevice_;
    vk::raii::Queue graphicsQueue_;
    vk::raii::Queue presentQueue_;
    vk::raii::CommandPool commandPool_;

    Device();
    void create(DeviceConfig config, const rendr::Window& win);
};

struct RendererSetup{
    vk::raii::DescriptorSetLayout descriptorSetLayout_;
    vk::raii::RenderPass renderPass_;
    vk::raii::PipelineLayout pipelineLayout_;
    vk::raii::Pipeline graphicsPipeline_;
    std::vector<vk::raii::Framebuffer> swapChainFramebuffers_;
    std::function<void(const rendr::Renderer&, rendr::RendererSetup& setup)> swapChainFramebuffersRecreationFunc_;
    vk::raii::DescriptorPool descriptorPool_;
    std::vector<vk::raii::DescriptorSet> descriptorSets_;

    RendererSetup();
};

struct RendererConfig{
    int framesInFlight = 2;
    DeviceConfig deviceConfig;
    SwapChainConfig swapChainConfig;
};

struct SwapChain{
    vk::raii::SwapchainKHR swapChain_;
    vk::Format swapChainImageFormat_;
    vk::Extent2D swapChainExtent_;
    std::vector<vk::Image> swapChainImages_;
    std::vector<vk::raii::ImageView> swapChainImageViews_;

    SwapChain();
    void create(const rendr::Device&  renderDevice, const rendr::Window& win, const rendr::SwapChainConfig& config);
    void clear();
};

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

struct DeviceWithGraphicsAndPresentQueues{
    vk::raii::Device device;
    vk::raii::Queue graphicsQueue;
    vk::raii::Queue presentQueue;
};

struct SwapChainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

struct SwapChainData{
    vk::raii::SwapchainKHR swapChain;
    vk::Format swapChainImageFormat;
    vk::Extent2D swapChainExtent;
};

class STBImageRaii {
private:
    stbi_uc* pixelsDataPtr;
    int width;
    int height;
    int texChannels;

public:
    STBImageRaii(const std::string& filePath)
        : pixelsDataPtr(nullptr), width(0), height(0), texChannels(0) {
        pixelsDataPtr = stbi_load(filePath.c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
        if (!pixelsDataPtr) {
            throw std::runtime_error("Failed to load texture image!");
        }
    }
    
    STBImageRaii(STBImageRaii&& other) noexcept
        : pixelsDataPtr(other.pixelsDataPtr), width(other.width), height(other.height), texChannels(other.texChannels) {
        other.pixelsDataPtr = nullptr;
        other.width = 0;
        other.height = 0;
        other.texChannels = 0;
    }

    STBImageRaii& operator=(STBImageRaii&& other) noexcept {
        if (this != &other) {
            std::swap(pixelsDataPtr, other.pixelsDataPtr);
            std::swap(width, other.width);
            std::swap(height, other.height);
            std::swap(texChannels, other.texChannels);
        }
        return *this;
    }

    ~STBImageRaii() {
        stbi_image_free(pixelsDataPtr);
    }

    STBImageRaii(const STBImageRaii&) = delete;
    STBImageRaii& operator=(const STBImageRaii&) = delete;

    stbi_uc* getDataPtr() const { return pixelsDataPtr; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getTexChannels() const { return texChannels; }
};

struct MVPUniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

struct PerFrameSync{
    vk::raii::Semaphore imageAvailableSemaphore;
    vk::raii::Semaphore renderFinishedSemaphore;
    vk::raii::Fence inFlightFence;
    PerFrameSync() : imageAvailableSemaphore(nullptr), renderFinishedSemaphore(nullptr), inFlightFence(nullptr) {}
};

class UfbxSceneRaii {
public:
    UfbxSceneRaii(const std::string& filepath) {
        ufbx_load_opts opts = { 0 };
        ufbx_error error;
        scene_ = ufbx_load_file(filepath.c_str(), &opts, &error);
        if (!scene_) {
            throw std::runtime_error(error.info);
        }
    }

    ~UfbxSceneRaii() {
        if (scene_) {
            ufbx_free_scene(scene_);
        }
    }

    UfbxSceneRaii(const UfbxSceneRaii&) = delete;
    UfbxSceneRaii& operator=(const UfbxSceneRaii&) = delete;

    UfbxSceneRaii(UfbxSceneRaii&& other) noexcept : scene_(other.scene_) {
        other.scene_ = nullptr;
    }

    UfbxSceneRaii& operator=(UfbxSceneRaii&& other) noexcept {
        if (this != &other) {
            if (scene_) {
                ufbx_free_scene(scene_);
            }
            scene_ = other.scene_;
            other.scene_ = nullptr;
        }
        return *this;
    }

    ufbx_scene* get() const {
        return scene_;
    }

private:
    ufbx_scene* scene_ = nullptr;
};

bool checkDeviceExtensionSupport(const vk::PhysicalDevice& device, const std::vector<const char*>& requiredExtensions);

bool isPhysicalDeviceSuitable(vk::raii::PhysicalDevice const & device, vk::raii::SurfaceKHR const & surface, const DeviceConfig& config);

vk::raii::PhysicalDevice pickPhysicalDevice(vk::raii::Instance const & instance, vk::raii::SurfaceKHR const & surface, const DeviceConfig& config);

QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface);

SwapChainSupportDetails querySwapChainSupport(vk::raii::PhysicalDevice const &device, vk::raii::SurfaceKHR const &surface);

DeviceWithGraphicsAndPresentQueues createDeviceWithGraphicsAndPresentQueues( vk::raii::PhysicalDevice const & physicalDevice,  vk::raii::SurfaceKHR const & surface, const DeviceConfig& config);

VkSurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const &availableFormats, const rendr::SwapChainConfig &config);

vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &availablePresentModes, const rendr::SwapChainConfig &config);

vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities, std::pair<int, int> const &winFramebufferSize);

SwapChainData createSwapChain(vk::raii::PhysicalDevice const &physicalDevice, vk::raii::SurfaceKHR const &surface, vk::raii::Device const &device, rendr::Window const &win, const rendr::SwapChainConfig &config);

vk::raii::ImageView createImageView(vk::raii::Device const &device, vk::Image const &image, vk::Format const &format, vk::ImageAspectFlags aspectFlags);

std::vector<vk::raii::ImageView> createImageViews(std::vector<vk::Image> const &images, vk::raii::Device const &device, vk::Format const &format, vk::ImageAspectFlags aspectFlags);

vk::Format findSupportedFormat(const vk::raii::PhysicalDevice &physicalDevice, std::vector<vk::Format> candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

vk::Format findDepthFormat(const vk::raii::PhysicalDevice &physicalDevice);

vk::raii::RenderPass createRenderPass(const vk::raii::Device &device, const std::vector<vk::AttachmentDescription> &attachments, const std::vector<vk::SubpassDescription> &subpasses, const std::vector<vk::SubpassDependency> &dependencies);

vk::raii::RenderPass createRenderPassWithColorAndDepthAttOneSubpass(const vk::raii::Device &device, vk::Format swapChainImageFormat, vk::Format depthFormat);

vk::raii::DescriptorSetLayout createDescriptorSetLayout(const vk::raii::Device &device, std::vector<vk::DescriptorSetLayoutBinding> bindings);

vk::raii::DescriptorSetLayout createSamplerDescriptorSetLayout(const vk::raii::Device &device);

vk::raii::DescriptorSetLayout createUboAndSamplerDescriptorSetLayout(const vk::raii::Device &device);

vk::raii::ShaderModule createShaderModule(const vk::raii::Device &device, const std::vector<char> &code);

vk::raii::PipelineLayout createPipelineLayout(const vk::raii::Device &device, const std::vector<vk::DescriptorSetLayout> &descriptorSetLayouts, const std::vector<vk::PushConstantRange> &pushConstantRanges);

uint32_t findMemoryType(vk::raii::PhysicalDevice const &physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties);

rendr::Image createImage(const vk::raii::PhysicalDevice &physicalDevice, const vk::raii::Device &device, vk::MemoryPropertyFlags properties, vk::ImageCreateInfo imageInfo, vk::ImageViewCreateInfo imageViewInfo);

rendr::Image createDepthImage(const vk::raii::PhysicalDevice &physicalDevice, const vk::raii::Device &device, uint32_t width, uint32_t height);

std::vector<vk::raii::Framebuffer> createSwapChainFramebuffersWithDepthAtt(const vk::raii::Device &device, const vk::raii::RenderPass &renderPass, const std::vector<vk::raii::ImageView> &swapChainImageViews, const vk::raii::ImageView &depthImageView, uint32_t width, uint32_t height);

vk::raii::CommandPool createGraphicsCommandPool(const vk::raii::Device &device, const rendr::QueueFamilyIndices &queueFamilyIndices);

rendr::Buffer createBuffer(const vk::raii::PhysicalDevice &physicalDevice, const vk::raii::Device &device, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);

vk::raii::CommandBuffer beginSingleTimeCommands(const vk::raii::Device &device, const vk::raii::CommandPool &commandPool);

void endSingleTimeCommands(const vk::raii::CommandBuffer &commandBuffer, const vk::raii::Queue &queueToSubmit);

void writeTransitionImageLayoutBarrier(const vk::raii::CommandBuffer &singleTimeCommandBuffer, const vk::raii::Image &image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

void writeCopyBufferToImageCommand(const vk::raii::CommandBuffer &singleTimeCommandBuffer, const vk::raii::Buffer &buffer, const vk::raii::Image &image, uint32_t width, uint32_t height);

Image create2DTextureImage(const rendr::Device &device, STBImageRaii ImageData);

Image create2DTextureImage(const vk::raii::PhysicalDevice &physicalDevice, const vk::raii::Device &device, const vk::raii::CommandPool &commandPool, const vk::raii::Queue &graphicsQueue, STBImageRaii ImageData);

vk::raii::Sampler createTextureSampler(const vk::raii::Device &device, const vk::raii::PhysicalDevice &physicalDevice);

std::pair<std::vector<VertexPTC>, std::vector<uint32_t>> loadModel(const std::string &filepath);

ufbx_scene *ufbxOpenScene(const std::string &filepath, bool blenderFlag);

void ufbxCloseScene(ufbx_scene *scene_ptr);

std::vector<std::pair<rendr::Mesh<VertexPTN>, uint32_t>> ufbxLoadMeshesPartsSepByMaterial(ufbx_scene *scene);

void writeCopyBufferCommand(const vk::raii::CommandBuffer &singleTimeCommandBuffer, const vk::raii::Buffer &srcBuffer, const vk::raii::Buffer &dstBuffer, vk::DeviceSize size);

rendr::Buffer createIndexBuffer(const rendr::Device& device, const std::vector<uint32_t> &indices);

rendr::Buffer createIndexBuffer(const vk::raii::PhysicalDevice &physicalDevice, const vk::raii::Device &device, const vk::raii::CommandPool &commandPool, const vk::raii::Queue &graphicsQueue, const std::vector<uint32_t> &indices);

std::vector<rendr::Buffer> createAndMapUniformBuffers(const vk::raii::PhysicalDevice &physicalDevice, const vk::raii::Device &device, std::vector<void *> &uniformBuffersMappedData, size_t numOfBuffers, MVPUniformBufferObject ubo);

vk::raii::DescriptorPool createDescriptorPool(const vk::raii::Device &device, uint32_t maxFramesInFlight);

std::vector<vk::raii::CommandBuffer> createCommandBuffers(const vk::raii::Device &device, const vk::raii::CommandPool &commandPool, uint32_t framesInFlight);

std::vector<rendr::PerFrameSync> createSyncObjects(const vk::raii::Device &device, uint32_t framesInFlight);

static std::vector<char> readFile(std::string const &filename){
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

template<typename VertexType>
rendr::Buffer createVertexBuffer(
    const rendr::Device& device,
    const std::vector<VertexType>& vertices){
    return createVertexBuffer<VertexType>(device.physicalDevice_, device.logicalDevice_, device.commandPool_, device.graphicsQueue_, vertices);  
}

template<typename VertexType>
rendr::Buffer createVertexBuffer(
    const vk::raii::PhysicalDevice &physicalDevice, 
    const vk::raii::Device &device,
    const vk::raii::CommandPool& commandPool,
    const vk::raii::Queue& graphicsQueue, 
    const std::vector<VertexType>& vertices){

    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    rendr::Buffer stagingBuffer = createBuffer(physicalDevice, device, bufferSize, vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eVertexBuffer, 
        vk::MemoryPropertyFlagBits::eHostVisible |  vk::MemoryPropertyFlagBits::eHostCoherent
    );

    void* data = stagingBuffer.bufferMemory.mapMemory(0,bufferSize);
        memcpy(data, vertices.data(), (size_t) bufferSize);
    stagingBuffer.bufferMemory.unmapMemory();


    rendr::Buffer vertexBuffer = createBuffer(physicalDevice, device, bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, 
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );

    vk::raii::CommandBuffer singleTimeCommandBuffer = rendr::beginSingleTimeCommands(device, commandPool);
        writeCopyBufferCommand(singleTimeCommandBuffer, stagingBuffer.buffer, vertexBuffer.buffer, bufferSize);
    rendr::endSingleTimeCommands(singleTimeCommandBuffer, graphicsQueue);
    
    return vertexBuffer;
}


inline vk::raii::Pipeline createGraphicsPipeline(
    const vk::raii::Device& device,
    const vk::raii::PipelineLayout& pipelineLayout,
    const vk::raii::RenderPass& renderPass,
    const std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages,
    const vk::PipelineVertexInputStateCreateInfo& vertexInputInfo,
    const vk::PipelineInputAssemblyStateCreateInfo& inputAssembly,
    const vk::PipelineViewportStateCreateInfo& viewportState,
    const vk::PipelineRasterizationStateCreateInfo& rasterizer,
    const vk::PipelineMultisampleStateCreateInfo& multisampling,
    const vk::PipelineColorBlendStateCreateInfo& colorBlending,
    const vk::PipelineDepthStencilStateCreateInfo& depthStencil,
    const vk::PipelineDynamicStateCreateInfo& dynamicState) {

    vk::GraphicsPipelineCreateInfo pipelineInfo(
        {}, // flags
        static_cast<uint32_t>(shaderStages.size()), // stageCount
        shaderStages.data(), // pStages
        &vertexInputInfo, // pVertexInputState
        &inputAssembly, // pInputAssemblyState
        nullptr, // pTessellationState
        &viewportState, // pViewportState
        &rasterizer, // pRasterizationState
        &multisampling, // pMultisampleState
        &depthStencil, // pDepthStencilState
        &colorBlending, // pColorBlendState
        &dynamicState, // pDynamicState
        *pipelineLayout, // layout
        *renderPass, // renderPass
        0, // subpass
        vk::Pipeline(), // basePipelineHandle
        -1 // basePipelineIndex
    );

    return vk::raii::Pipeline(device, nullptr, pipelineInfo);
}

template<typename VertexType>
vk::raii::Pipeline createGraphicsPipelineWithDefaults(
    const vk::raii::Device& device,
    const vk::raii::RenderPass& renderPass,
    const vk::raii::PipelineLayout& pipelineLayout,
    vk::Extent2D swapChainExtent, VertexType vertexType, 
    const vk::raii::ShaderModule& vertShaderModule,
    const vk::raii::ShaderModule& fragShaderModule) {

    vk::PipelineShaderStageCreateInfo vertShaderStageInfo(
        {}, // flags
        vk::ShaderStageFlagBits::eVertex, // stage
        *vertShaderModule, // module
        "main" // pName
    );

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo(
        {}, // flags
        vk::ShaderStageFlagBits::eFragment, // stage
        *fragShaderModule, // module
        "main" // pName
    );

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

    auto bindingDescription = VertexType::getBindingDescription();
    auto attributeDescriptions = VertexType::getAttributeDescriptions();

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo(
        {}, // flags
        1, // vertexBindingDescriptionCount
        &bindingDescription, // pVertexBindingDescriptions
        static_cast<uint32_t>(attributeDescriptions.size()), // vertexAttributeDescriptionCount
        attributeDescriptions.data() // pVertexAttributeDescriptions
    );

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly(
        {}, // flags
        vk::PrimitiveTopology::eTriangleList, // topology
        VK_FALSE // primitiveRestartEnable
    );

    vk::Viewport viewport(
        0.0f, // x
        0.0f, // y
        static_cast<float>(swapChainExtent.width), // width
        static_cast<float>(swapChainExtent.height), // height
        0.0f, // minDepth
        1.0f // maxDepth
    );

    vk::Rect2D scissor({ 0, 0 }, swapChainExtent); // offset, extent

    vk::PipelineViewportStateCreateInfo viewportState(
        {}, // flags
        1, // viewportCount
        &viewport, // pViewports
        1, // scissorCount
        &scissor // pScissors
    );

    vk::PipelineRasterizationStateCreateInfo rasterizer(
        {}, // flags
        VK_FALSE, // depthClampEnable
        VK_FALSE, // rasterizerDiscardEnable
        vk::PolygonMode::eFill, // polygonMode
        vk::CullModeFlagBits::eBack, // cullMode
        vk::FrontFace::eCounterClockwise, // frontFace
        VK_FALSE, // depthBiasEnable
        0.0f, // depthBiasConstantFactor
        0.0f, // depthBiasClamp
        0.0f, // depthBiasSlopeFactor
        1.0f // lineWidth
    );

    vk::PipelineMultisampleStateCreateInfo multisampling(
        {}, // flags
        vk::SampleCountFlagBits::e1, // rasterizationSamples
        VK_FALSE, // sampleShadingEnable
        1.0f, // minSampleShading
        nullptr, // pSampleMask
        VK_FALSE, // alphaToCoverageEnable
        VK_FALSE // alphaToOneEnable
    );

    vk::PipelineColorBlendAttachmentState colorBlendAttachment(
        VK_TRUE, // blendEnable
        vk::BlendFactor::eSrcAlpha, // srcColorBlendFactor
        vk::BlendFactor::eOneMinusSrcAlpha, // dstColorBlendFactor
        vk::BlendOp::eAdd, // colorBlendOp
        vk::BlendFactor::eOne, // srcAlphaBlendFactor
        vk::BlendFactor::eZero, // dstAlphaBlendFactor
        vk::BlendOp::eAdd, // alphaBlendOp
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA 
    );

    vk::PipelineColorBlendStateCreateInfo colorBlending(
        {}, // flags
        VK_FALSE, // logicOpEnable
        vk::LogicOp::eCopy, // logicOp
        1, // attachmentCount
        &colorBlendAttachment, // pAttachments
        { 0.0f, 0.0f, 0.0f, 0.0f } // blendConstants
    );

    vk::StencilOpState stencilOpState(
        vk::StencilOp::eKeep, // failOp
        vk::StencilOp::eKeep, // passOp
        vk::StencilOp::eKeep, // depthFailOp
        vk::CompareOp::eAlways, // compareOp
        0, // compareMask
        0, // writeMask
        0 // reference
    );

    vk::PipelineDepthStencilStateCreateInfo depthStencil(
        {}, // flags
        VK_TRUE, // depthTestEnable
        VK_TRUE, // depthWriteEnable
        vk::CompareOp::eLess, // depthCompareOp
        VK_FALSE, // depthBoundsTestEnable
        VK_FALSE, // stencilTestEnable
        stencilOpState, // front
        stencilOpState, // back
        0.0f, // minDepthBounds
        1.0f // maxDepthBounds
    );

    std::vector<vk::DynamicState> dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamicState(
        {}, // flags
        static_cast<uint32_t>(dynamicStates.size()), // dynamicStateCount
        dynamicStates.data() // pDynamicStates
    );

    return createGraphicsPipeline(
        device,
        pipelineLayout,
        renderPass,
        shaderStages,
        vertexInputInfo,
        inputAssembly,
        viewportState,
        rasterizer,
        multisampling,
        colorBlending,
        depthStencil,
        dynamicState
    );
}

struct Camera
{   
    constexpr static const glm::vec3 worldUp{0.0f, 1.0f, 0.0f};
    glm::vec3 pos{1.0f, 1.0f, 1.0f};
    glm::vec3 look_at_pos{0.0f, 0.0f, 0.0f};

    float fov = 45.0f;
    float aspect = 16.0f / 9.0f;
    float near_plane = 0.1f;
    float far_plane = 1000.0f;

    rendr::MVPUniformBufferObject getMVPubo(float scale, float cameraAspect){
        rendr::MVPUniformBufferObject ubo;
        ubo.model = glm::scale(glm::mat4{1.0f}, glm::vec3(scale, scale, scale));
        //ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        ubo.view = glm::lookAt(pos, look_at_pos, worldUp);
        ubo.proj = glm::perspective(fov, cameraAspect, near_plane, far_plane);
        ubo.proj[1][1] *= -1;
        return ubo;
    }
};

template<typename VertexType>
std::map<uint32_t, rendr::Mesh<VertexType>> mergeMeshesByMaterial(const std::vector<std::pair<rendr::Mesh<VertexType>, uint32_t>>& meshesAndMatInd) {
    
    std::map<uint32_t, rendr::Mesh<VertexType>> materialToMesh;

    // Подсчет общего количества вершин и индексов для каждого материала
    std::map<uint32_t, size_t> materialVertexCount;
    std::map<uint32_t, size_t> materialIndexCount;
    for (const auto& meshMatPair : meshesAndMatInd) {
        uint32_t materialIndex = meshMatPair.second;
        materialVertexCount[materialIndex] += meshMatPair.first.vertices.size();
        materialIndexCount[materialIndex] += meshMatPair.first.indices.size();
    }

    // Резервирование памяти
    for (const auto& entry : materialVertexCount) {
        materialToMesh[entry.first].vertices.reserve(entry.second);
    }
    for (const auto& entry : materialIndexCount) {
        materialToMesh[entry.first].indices.reserve(entry.second);
    }

    // Объединение вершин и индексов по материалам
    for (const auto& meshMatPair : meshesAndMatInd) {
        const auto& mesh = meshMatPair.first;
        uint32_t materialIndex = meshMatPair.second;

        auto& meshData = materialToMesh[materialIndex];
        auto vertexOffset = meshData.vertices.size();

        meshData.vertices.insert(meshData.vertices.end(), mesh.vertices.begin(), mesh.vertices.end());

        // Обновление индексов с учетом смещения
        for (auto index : mesh.indices) {
            meshData.indices.push_back(index + vertexOffset);
        }
    }

    return materialToMesh;
}

std::vector<vk::raii::DescriptorSet> createDescriptorSets(    
    const vk::raii::Device& device,  
    const vk::raii::DescriptorPool& descriptorPool,
    const vk::raii::DescriptorSetLayout& descriptorSetLayout, 
    int maxFramesInFlight
);

vk::raii::DescriptorSetLayout createUboDescriptorSetLayout(const vk::raii::Device &device);

class IDrawableObj;
class ISetupBinder;
class DrawableObj;

class Renderer{
private:
    int framesInFlight_;  
    int currentFrame_ = 0;
    int setupsIndCount_ = 0;
    rendr::Device device_;
    rendr::SwapChain swapChain_;
    rendr::SwapChainConfig swapChainConfig_;
    rendr::Image depthImage_;
    std::map<int, rendr::RendererSetup> rendrSetups_;
    std::vector<vk::raii::CommandBuffer> commandBuffers_;
    std::vector<rendr::PerFrameSync> framesSyncObjs_;
    std::vector<rendr::Buffer> uniformBuffers_;
    std::vector<void*> uniformBuffersMapped_;

    vk::raii::DescriptorSetLayout descriptorSetLayout_;
    vk::raii::DescriptorPool descriptorPool_;
    std::vector<vk::raii::DescriptorSet> descriptorSets_;

    std::map<int, std::vector<rendr::IDrawableObj*>> setupIndexToDrawableObjs;

    void cleanupSwapChain();
    void recordCommandBuffer(uint32_t imageIndex); 
public:
    Renderer();
    void drawFrame();
    void setDrawableObjects(std::vector<IDrawableObj*> objs);
    void initRenderSetup(ISetupBinder& material);
    void init(const RendererConfig& config, rendr::Window& win);
    void recreateSwapChain(const rendr::Window& window);
    void waitIdle();
    void updateMVPUniformBuffer(rendr::MVPUniformBufferObject ubo);
    float getSwapChainAspect();
    
    
    const rendr::RendererSetup& getRenderSetup(int setupIndex) const{
        return rendrSetups_.at(setupIndex);
    }

    const rendr::Device& getDevice() const{
        return device_;
    }

    const rendr::SwapChain& getSwapChain() const{
        return swapChain_;
    }
    
    const rendr::Image& getDepthImage() const{
        return depthImage_;
    }

    const int getNumOfFramesInFlight() const{
        return framesInFlight_;
    }
};

struct ISetupBinder{
    int renderSetupIndex = -1;
    virtual RendererSetup createRendererSetup(const rendr::Renderer& renderer,
        const vk::raii::DescriptorSetLayout& rendererDescriptorSetLayout,
        int framesInFlight) = 0;
    virtual void bindSetupResources(const vk::raii::Device& device, const vk::raii::CommandBuffer& buffer, const vk::raii::PipelineLayout& layout, int curFrame) = 0;
    virtual ~ISetupBinder(){};
};

struct IDrawableObj {
    IDrawableObj(ISetupBinder& binder) : setupBinder(&binder){}
    ISetupBinder* setupBinder;
    virtual void bindObjResources(const vk::raii::Device& device, const vk::raii::CommandBuffer& buffer, const vk::raii::PipelineLayout& layout, int curFrame) = 0;
    virtual size_t getNumOfDrawIndices() = 0;
    virtual ~IDrawableObj(){};
};

enum class LightType{
    DIRECTIONAL,
    SPOT,
    POINT
};

struct Light{
    glm::vec3 pos;
    LightType type;
    glm::vec3 direction;
    glm::vec3 color;
    float intesity;
    float cutoff;
};


}