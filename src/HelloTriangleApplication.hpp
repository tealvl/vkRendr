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
#include "inputManager.hpp"
#include "CameraManipulator.hpp"
#include "timer.hpp"

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
    descriptorPool_(nullptr),
    framebufferResized(false)
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

        std::vector<char> vertShaderCode = rendr::readFile("C:/Dev/cpp-projects/engine/src/shaders/fvertex.spv");
        std::vector<char> fragShaderCode = rendr::readFile("C:/Dev/cpp-projects/engine/src/shaders/ffragment.spv");
        vk::raii::ShaderModule vertShaderModule = rendr::createShaderModule(device_, vertShaderCode);
        vk::raii::ShaderModule fragShaderModule = rendr::createShaderModule(device_, fragShaderCode);

        graphicsPipeline_ = rendr::createGraphicsPipelineWithDefaults(device_, renderPass_, pipelineLayout_, swapChainExtent_, rendr::VertexPTN{},
            vertShaderModule, fragShaderModule
        );

        depthImage_ = rendr::createDepthImage(physicalDevice_, device_, swapChainExtent_.width, swapChainExtent_.height);
        swapChainFramebuffers_ = rendr::createSwapChainFramebuffersWithDepthAtt(device_, renderPass_, swapChainImageViews_, depthImage_.imageView, swapChainExtent_.width, swapChainExtent_.height);
        commandPool_ =  rendr::createGraphicsCommandPool(device_, rendr::findQueueFamilies(*physicalDevice_, *surface_));
        
        textureSampler_ = rendr::createTextureSampler(device_, physicalDevice_);


        rendr::UfbxSceneRaii fbxScene("C:/Dev/cpp-projects/engine/resources/zen-studio/source/zen_studio.fbx");
        auto meshesAndMatInd = rendr::ufbxLoadMeshesPartsSepByMaterial(fbxScene.get());  
        auto matToMesh = rendr::mergeMeshesByMaterial(meshesAndMatInd);
        
        meshesAndMatInd.clear();
        rendr::STBImageRaii walls("C:/Dev/cpp-projects/engine/resources/zen-studio/textures/t_walls_baked.png");  
        rendr::STBImageRaii details("C:/Dev/cpp-projects/engine/resources/zen-studio/textures/t_details_Baked.png");  
        rendr::Image wallsTextureImage = rendr::create2DTextureImage(physicalDevice_, device_, commandPool_, graphicsQueue_, std::move(walls));
        rendr::Image detailsTextureImage = rendr::create2DTextureImage(physicalDevice_, device_, commandPool_, graphicsQueue_, std::move(details));
        rendr::SimpleMaterial wallsMat (0, std::move(wallsTextureImage));
        rendr::SimpleMaterial detailsMat (2, std::move(detailsTextureImage));

        simpleMaterials_.push_back(std::move(wallsMat));
        simpleMaterials_.push_back(std::move(detailsMat));
        singleSimpleMatMeshes_.push_back(std::move(matToMesh[0]));
        singleSimpleMatMeshes_.push_back(std::move(matToMesh[2]));


        batches_.push_back(
        rendr::Batch(
            rendr::createVertexBuffer(physicalDevice_, device_, commandPool_, graphicsQueue_, singleSimpleMatMeshes_[0].vertices),
            rendr::createIndexBuffer(physicalDevice_, device_, commandPool_, graphicsQueue_, singleSimpleMatMeshes_[0].indices),
            &simpleMaterials_[0]
        ));
        batches_.push_back(
        rendr::Batch(
            rendr::createVertexBuffer(physicalDevice_, device_, commandPool_, graphicsQueue_, singleSimpleMatMeshes_[1].vertices),
            rendr::createIndexBuffer(physicalDevice_, device_, commandPool_, graphicsQueue_, singleSimpleMatMeshes_[1].indices),
            &simpleMaterials_[1]
        ));
       
        // vertices = meshesAndMatInd[0].first.vertices;
        // indices = meshesAndMatInd[0].first.indices;
        // textureImage_ = rendr::create2DTextureImage(physicalDevice_, device_, commandPool_, graphicsQueue_, std::move(walls));
        // vertexBuffer_ = rendr::createVertexBuffer(physicalDevice_, device_, commandPool_, graphicsQueue_, vertices);
        // indexBuffer_ = rendr::createIndexBuffer(physicalDevice_, device_, commandPool_, graphicsQueue_, indices);

        uniformBuffers_ = rendr::createAndMapUniformBuffers(physicalDevice_, device_, uniformBuffersMapped_, FramesInFlight, rendr::MVPUniformBufferObject());
        descriptorPool_ = rendr::createDescriptorPool(device_, FramesInFlight, batches_.size());
        descriptorSets_ = rendr::createUboAndSamplerDescriptorSets(device_, descriptorPool_, descriptorSetLayout_, uniformBuffers_,
            textureSampler_, FramesInFlight, batches_, rendr::MVPUniformBufferObject()
        );
        commandBuffers_ = rendr::createCommandBuffers(device_, commandPool_, FramesInFlight);
        framesSyncObjs_ = rendr::createSyncObjects(device_, FramesInFlight);


        window_.callbacks.winResized = [this](int,int){
            this->framebufferResized = true;
            this->recreateSwapChain();
        };

        inputManager_.setUpWindowCallbacks(window_);
        camManip_.setCamera(camera_);
        camManip_.setInputManager(inputManager_);
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
    bool framebufferResized;

    rendr::Buffer vertexBuffer_;
    rendr::Buffer indexBuffer_;

    std::vector<rendr::Buffer> uniformBuffers_;
    std::vector<void*> uniformBuffersMapped_;

    vk::raii::DescriptorPool descriptorPool_;
    std::vector<std::vector<vk::raii::DescriptorSet>> descriptorSets_;
    
    rendr::Image textureImage_;
    vk::raii::Sampler textureSampler_;

    std::vector<rendr::VertexPTN> vertices;
    std::vector<uint32_t> indices;

    std::vector<rendr::SimpleMaterial> simpleMaterials_;
    std::vector<rendr::Mesh<rendr::VertexPTN>> singleSimpleMatMeshes_;
    std::vector<rendr::Batch<rendr::SimpleMaterial>> batches_;
    rendr::InputManager inputManager_;
    rendr::CameraManipulator camManip_;
    rendr::Camera camera_;
    rendr::Transform model_matrix_;
    Timer timer_;
    
    void mainLoop();
    void cleanupSwapChain();
    void recreateSwapChain();
    void recordCommandBuffer(const vk::raii::CommandBuffer &commandBuffer, uint32_t imageIndex);
    void drawFrame();
    void updateUniformBuffer(uint32_t currentImage);
};